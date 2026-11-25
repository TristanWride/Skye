#include "skye/Event.hpp"

#include <algorithm>

namespace skye {

EventType EventSystem::GenerateEventType() {
  return eventTypes_.CreateID();
}

EventSystem::ListenerID EventSystem::RegisterListener( Listener * listener ) {
  const ListenerID listenerID = listenerIDs_.CreateID();

  auto lock = std::lock_guard{ listenersMutex_ };
  listeners_.emplace( listenerID, listener );
  return listenerID;
}

void EventSystem::RegisterListenerToChannel( ListenerID listenerID, ChannelID channelID ) {
  auto lock = std::lock_guard{ listenersMutex_ };
  channels_[channelID].push_back( listenerID );
}

void EventSystem::UnregisterListenerFromChannel( ListenerID listenerID, ChannelID channelID ) {
  auto lock = std::lock_guard{ listenersMutex_ };

  const auto channel = channels_.find( channelID );
  if ( channel == channels_.end() ) return;

  std::erase( channel->second, listenerID );
  if ( channel->second.empty() ) channels_.erase( channel );
}

void EventSystem::UnregisterListener( ListenerID listenerID ) {
  // Blocks until other threads finish ProcessEvents(), so the caller may destroy the listener afterwards.
  auto dispatchLock = std::lock_guard{ dispatchMutex_ };
  auto lock         = std::lock_guard{ listenersMutex_ };

  listenerIDs_.DestroyID( listenerID );
  listeners_.erase( listenerID );

  // IDs are reused, so a stale channel entry would send events to the next listener given this ID.
  std::erase_if( channels_, [&]( auto & channel ) {
    std::erase( channel.second, listenerID );
    return channel.second.empty();
  } );
}

void EventSystem::QueueEvent( Event event ) {
  auto lock = std::lock_guard{ queueMutex_ };
  eventQueue_.push_back( event );
}

void EventSystem::ProcessEvents() {
  auto dispatchLock = std::lock_guard{ dispatchMutex_ };

  // Take the whole queue first, so listeners that queue events from OnEvent can't extend this loop forever.
  auto events = std::deque<Event>{};
  {
    auto lock = std::lock_guard{ queueMutex_ };
    events.swap( eventQueue_ );
  }

  for ( const Event & event : events )
    DispatchEvent( event );
}

void EventSystem::DispatchEvent( const Event & event ) {
  // Collect recipient IDs under the lock, then call them without it, so OnEvent can use the event system.
  auto recipients = std::vector<ListenerID>{};
  {
    auto lock = std::lock_guard{ listenersMutex_ };

    switch ( event.target.type ) {
      case EventTarget::Type::Broadcast:
        recipients.reserve( listeners_.size() );
        for ( const auto & listener : listeners_ )
          recipients.push_back( listener.first );
        break;

      case EventTarget::Type::Channel: {
        const auto channel = channels_.find( event.target.channel.channelID );
        if ( channel != channels_.end() ) recipients = channel->second;
        break;
      }

      case EventTarget::Type::Single:
        recipients.push_back( event.target.single.listenerID );
        break;
    }
  }

  for ( const ListenerID listenerID : recipients )
    NotifyListener( listenerID, event );
}

void EventSystem::NotifyListener( ListenerID listenerID, const Event & event ) {
  Listener * listener = nullptr;
  {
    auto lock = std::lock_guard{ listenersMutex_ };

    // The listener may have been unregistered after the event was queued.
    const auto found = listeners_.find( listenerID );
    if ( found == listeners_.end() ) return;
    listener = found->second;
  }

  // Safe to call unlocked: UnregisterListener() can't complete while we hold dispatchMutex_.
  listener->OnEvent( event.eventType, event.target.payload );
}

}

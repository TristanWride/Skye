#include "skye/Event.hpp"

#include <algorithm>

namespace skye {

EventType EventSystem::GenerateEventType() {
  return eventTypes_.CreateID();
}

EventSystem::ListenerID EventSystem::RegisterListener( Listener * listener ) {
  const ListenerID listenerID = listenerIDs_.CreateID();
  listeners_.emplace( listenerID, listener );
  return listenerID;
}

void EventSystem::RegisterListenerToChannel( ListenerID listenerID, ChannelID channelID ) {
  channels_[channelID].push_back( listenerID );
}

void EventSystem::UnregisterListenerFromChannel( ListenerID listenerID, ChannelID channelID ) {
  const auto channel = channels_.find( channelID );
  if ( channel == channels_.end() ) return;

  std::erase( channel->second, listenerID );
  if ( channel->second.empty() ) channels_.erase( channel );
}

void EventSystem::UnregisterListener( ListenerID listenerID ) {
  listenerIDs_.DestroyID( listenerID );
  listeners_.erase( listenerID );

  // A stale channel entry could never match a later listener (it gets a new generation), but would linger forever.
  std::erase_if( channels_, [&]( auto & channel ) {
    std::erase( channel.second, listenerID );
    return channel.second.empty();
  } );
}

void EventSystem::QueueEvent( Event event ) {
  eventQueue_.push_back( event );
}

void EventSystem::ProcessEvents() {
  // Take the whole queue first, so listeners that queue events from OnEvent can't extend this loop forever,
  // and so a nested ProcessEvents() from inside OnEvent can't invalidate this loop's iterators.
  auto events = std::deque<Event>{};
  events.swap( eventQueue_ );

  for ( const Event & event : events )
    DispatchEvent( event );
}

void EventSystem::DispatchEvent( const Event & event ) {
  // Copy the recipient IDs before calling any of them: OnEvent may register or unregister listeners, which
  // would invalidate iterators into listeners_ or channels_.
  auto recipients = std::vector<ListenerID>{};
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

  for ( const ListenerID listenerID : recipients )
    NotifyListener( listenerID, event );
}

void EventSystem::NotifyListener( ListenerID listenerID, const Event & event ) {
  // Look the listener up again: it may have been unregistered after the event was queued, or by an earlier
  // recipient's OnEvent during this dispatch. listeners_ is keyed by the full ID, generation included, so a stale ID
  // misses even when its index now belongs to another listener.
  const auto found = listeners_.find( listenerID );
  if ( found == listeners_.end() ) return;

  found->second->OnEvent( event.eventType, event.target.payload );
}

}

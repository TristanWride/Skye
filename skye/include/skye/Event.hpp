#pragma once

#include "skye/IDAllocator.hpp"

#include <cstdint>
#include <deque>
#include <map>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace skye {

using EventID = uint32_t;

constexpr static EventID NULL_EVENT = 0u;

class Listener {
public:
  virtual ~Listener() = default;

  // Called by EventSystem::ProcessEvents() on the thread that is processing events.
  virtual void OnEvent( EventID eventID, void * eventData ) = 0;
};

// Thread-safe. Every function may be called from any thread, including from inside Listener::OnEvent.
class EventSystem {
public:
  using size_type  = uint64_t;
  using ListenerID = uint32_t;
  using ChannelID  = uint32_t;

  struct EventTarget {
    enum class Type : uint8_t {
      Broadcast,
      Channel,
      Single
    } type;

    union {
      struct {
        ChannelID channelID;
      } channel;

      struct {
        ListenerID listenerID;
      } single;
    };

    void * payload;
  };

  // TODO: Rename eventID to eventType
  struct Event {
    EventID     eventID;
    EventTarget target;
  };

private:
  // Lock order: dispatchMutex_, then listenersMutex_. queueMutex_ is never held with another lock.
  // Held for all of ProcessEvents(); UnregisterListener() takes it so it waits for in-flight callbacks.
  // Recursive so a listener can unregister (or process events) from inside its own OnEvent.
  std::recursive_mutex dispatchMutex_;
  std::mutex           listenersMutex_; // Guards listeners_ and channels_.
  std::mutex           queueMutex_;     // Guards eventQueue_.

  IDAllocator<EventID>                                   eventIDs_{ NULL_EVENT + 1 };
  IDAllocator<ListenerID>                                listenerIDs_{ 1 };
  std::deque<Event>                                      eventQueue_{};
  std::map<ListenerID, Listener *>                       listeners_{}; // Ordered: broadcasts go out in ID order.
  std::unordered_map<ChannelID, std::vector<ListenerID>> channels_{};

public:
  // Returns a new unique event ID. Never returns NULL_EVENT.
  [[nodiscard]] EventID GenerateEventID();

  // Registers a listener and returns its ID (never 0), needed to unregister it.
  [[nodiscard]] ListenerID RegisterListener( Listener * listener );

  // Adds a listener to a channel, creating the channel if needed.
  void RegisterListenerToChannel( ListenerID listenerID, ChannelID channelID );

  // Removes a listener from one channel. An event already being dispatched may still reach it.
  void UnregisterListenerFromChannel( ListenerID listenerID, ChannelID channelID );

  // Removes a listener from every channel and frees its ID for reuse.
  // Waits for ProcessEvents() on other threads: once this returns, the listener is never called again.
  void UnregisterListener( ListenerID listenerID );

  // Adds an event to the queue. It is delivered on the next ProcessEvents() call.
  void QueueEvent( Event event );

  // Delivers every queued event to its target. Events queued while processing wait for the next call.
  void ProcessEvents();

private:
  // Sends one event to its broadcast, channel or single target.
  void DispatchEvent( const Event & event );

  // Calls OnEvent on one listener, if it is still registered. Requires dispatchMutex_ to be held.
  void NotifyListener( ListenerID listenerID, const Event & event );
};

}

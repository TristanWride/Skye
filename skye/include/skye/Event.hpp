#pragma once

#include "skye/IDAllocator.hpp"

#include <cstdint>
#include <deque>
#include <map>
#include <unordered_map>
#include <vector>

namespace skye {

using EventType = uint32_t;

constexpr static EventType NULL_EVENT_TYPE = 0u;

class Listener {
public:
  virtual ~Listener() = default;

  // Called by EventSystem::ProcessEvents() for each event delivered to this listener.
  virtual void OnEvent( EventType eventType, void * eventData ) = 0;
};

// Not thread-safe: use it from one thread, or synchronise access externally.
// Every function may still be called from inside Listener::OnEvent.
class EventSystem {
public:
  using size_type  = uint64_t;
  using ListenerID = uint32_t;
  using ChannelID  = uint32_t;
  // Generational, so a stale listener ID stops matching once its index is reused.
  using ListenerIDAllocator = IDAllocator<ListenerID, 8>;

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

  struct Event {
    EventType   eventType;
    EventTarget target;
  };

private:
  IDAllocator<EventType>                                 eventTypes_{ NULL_EVENT_TYPE + 1 };
  ListenerIDAllocator                                    listenerIDs_{ 1 };
  std::deque<Event>                                      eventQueue_{};
  std::map<ListenerID, Listener *>                       listeners_{}; // Ordered: broadcasts go out in full ID order.
  std::unordered_map<ChannelID, std::vector<ListenerID>> channels_{};

public:
  // Returns a new unique event type. Never returns NULL_EVENT_TYPE.
  [[nodiscard]] EventType GenerateEventType();

  // Registers a listener and returns its ID (never 0), needed to unregister it.
  [[nodiscard]] ListenerID RegisterListener( Listener * listener );

  // Adds a listener to a channel, creating the channel if needed.
  void RegisterListenerToChannel( ListenerID listenerID, ChannelID channelID );

  // Removes a listener from one channel. An event already being dispatched may still reach it.
  void UnregisterListenerFromChannel( ListenerID listenerID, ChannelID channelID );

  // Removes a listener from every channel and frees its ID's index for reuse under a new generation. Once this
  // returns, the listener is never called again, even by an event that is part-way through being dispatched, and
  // events still addressed to its ID do not reach a listener that later reuses the index (barring generation
  // wrap-around after 256 reuses of one index).
  void UnregisterListener( ListenerID listenerID );

  // Adds an event to the queue. It is delivered on the next ProcessEvents() call.
  void QueueEvent( Event event );

  // Delivers every queued event to its target. Events queued while processing wait for the next call.
  void ProcessEvents();

private:
  // Sends one event to its broadcast, channel or single target.
  void DispatchEvent( const Event & event );

  // Calls OnEvent on one listener, if it is still registered.
  void NotifyListener( ListenerID listenerID, const Event & event );
};

}

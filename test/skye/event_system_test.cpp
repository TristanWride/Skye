#include "skye/Event.hpp"
#include "skye/testing/Testing.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>
#include <tuple>
#include <vector>

namespace {

using skye::EventSystem;

// Records every event it receives; optionally runs a callback from inside OnEvent.
class RecordingListener : public skye::Listener {
public:
  std::vector<skye::EventType>                     received;
  std::vector<void *>                              payloads;
  std::function<void( skye::EventType eventType )> onEvent;

public:
  void OnEvent( skye::EventType eventType, void * eventData ) override {
    received.push_back( eventType );
    payloads.push_back( eventData );
    if ( onEvent ) onEvent( eventType );
  }
};

EventSystem::Event MakeBroadcast( skye::EventType eventType ) {
  auto event        = EventSystem::Event{};
  event.eventType   = eventType;
  event.target.type = EventSystem::EventTarget::Type::Broadcast;
  return event;
}

EventSystem::Event MakeChannelEvent( skye::EventType eventType, EventSystem::ChannelID channelID ) {
  auto event                     = EventSystem::Event{};
  event.eventType                = eventType;
  event.target.type              = EventSystem::EventTarget::Type::Channel;
  event.target.channel.channelID = channelID;
  return event;
}

EventSystem::Event MakeSingleEvent( skye::EventType eventType, EventSystem::ListenerID listenerID ) {
  auto event                     = EventSystem::Event{};
  event.eventType                = eventType;
  event.target.type              = EventSystem::EventTarget::Type::Single;
  event.target.single.listenerID = listenerID;
  return event;
}

}

TEST( EventSystemTest, BroadcastReachesEveryListener ) {
  auto events = EventSystem{};
  auto a      = RecordingListener{};
  auto b      = RecordingListener{};
  std::ignore = events.RegisterListener( &a );
  std::ignore = events.RegisterListener( &b );

  events.QueueEvent( MakeBroadcast( 7 ) );
  events.ProcessEvents();

  EXPECT_EQ( a.received, std::vector<skye::EventType>{ 7 } );
  EXPECT_EQ( b.received, std::vector<skye::EventType>{ 7 } );
}

TEST( EventSystemTest, ChannelReachesOnlyMembers ) {
  auto                          events = EventSystem{};
  auto                          member = RecordingListener{};
  auto                          other  = RecordingListener{};
  const EventSystem::ListenerID id     = events.RegisterListener( &member );
  std::ignore                          = events.RegisterListener( &other );
  events.RegisterListenerToChannel( id, 3 );

  events.QueueEvent( MakeChannelEvent( 7, 3 ) );
  events.QueueEvent( MakeChannelEvent( 8, 4 ) );
  events.ProcessEvents();

  EXPECT_EQ( member.received, std::vector<skye::EventType>{ 7 } );
  EXPECT_TRUE( other.received.empty() );
}

TEST( EventSystemTest, SingleReachesOnlyTargetWithPayload ) {
  auto                          events = EventSystem{};
  auto                          target = RecordingListener{};
  auto                          other  = RecordingListener{};
  const EventSystem::ListenerID id     = events.RegisterListener( &target );
  std::ignore                          = events.RegisterListener( &other );
  int payload                          = 42;

  auto event           = MakeSingleEvent( 7, id );
  event.target.payload = &payload;
  events.QueueEvent( event );
  events.ProcessEvents();

  ASSERT_EQ( target.payloads.size(), 1u );
  EXPECT_EQ( target.payloads[0], &payload );
  EXPECT_TRUE( other.received.empty() );
}

TEST( EventSystemTest, EventsQueuedDuringProcessingWaitForNextCall ) {
  auto events      = EventSystem{};
  auto listener    = RecordingListener{};
  std::ignore      = events.RegisterListener( &listener );
  listener.onEvent = [&]( skye::EventType eventType ) {
    if ( eventType == 1 ) events.QueueEvent( MakeBroadcast( 2 ) );
  };

  events.QueueEvent( MakeBroadcast( 1 ) );
  events.ProcessEvents();
  EXPECT_EQ( listener.received, std::vector<skye::EventType>{ 1 } );

  events.ProcessEvents();
  EXPECT_EQ( listener.received, ( std::vector<skye::EventType>{ 1, 2 } ) );
}

TEST( EventSystemTest, UnregisteredListenerReceivesNothing ) {
  auto                          events   = EventSystem{};
  auto                          listener = RecordingListener{};
  const EventSystem::ListenerID id       = events.RegisterListener( &listener );

  events.QueueEvent( MakeSingleEvent( 7, id ) );
  events.UnregisterListener( id );
  events.ProcessEvents();

  EXPECT_TRUE( listener.received.empty() );
}

TEST( EventSystemTest, ReusedIDDoesNotInheritChannels ) {
  auto                          events = EventSystem{};
  auto                          first  = RecordingListener{};
  auto                          second = RecordingListener{};
  const EventSystem::ListenerID id     = events.RegisterListener( &first );
  events.RegisterListenerToChannel( id, 3 );
  events.UnregisterListener( id );

  const EventSystem::ListenerID reusedID = events.RegisterListener( &second );
  ASSERT_EQ( reusedID, id );

  events.QueueEvent( MakeChannelEvent( 7, 3 ) );
  events.ProcessEvents();

  EXPECT_TRUE( second.received.empty() );
}

TEST( EventSystemTest, BroadcastGoesOutInListenerIDOrder ) {
  auto events    = EventSystem{};
  auto order     = std::vector<int>{};
  auto first     = RecordingListener{};
  auto second    = RecordingListener{};
  first.onEvent  = [&]( skye::EventType ) { order.push_back( 1 ); };
  second.onEvent = [&]( skye::EventType ) { order.push_back( 2 ); };
  std::ignore    = events.RegisterListener( &first );
  std::ignore    = events.RegisterListener( &second );

  events.QueueEvent( MakeBroadcast( 7 ) );
  events.ProcessEvents();

  EXPECT_EQ( order, ( std::vector<int>{ 1, 2 } ) );
}

TEST( EventSystemTest, ListenerCanUnregisterItselfDuringOnEvent ) {
  auto                    events   = EventSystem{};
  auto                    listener = RecordingListener{};
  EventSystem::ListenerID id       = events.RegisterListener( &listener );
  listener.onEvent                 = [&]( skye::EventType ) { events.UnregisterListener( id ); };

  events.QueueEvent( MakeSingleEvent( 7, id ) );
  events.QueueEvent( MakeSingleEvent( 8, id ) );
  events.ProcessEvents();

  EXPECT_EQ( listener.received, std::vector<skye::EventType>{ 7 } );
}

TEST( EventSystemTest, ConcurrentQueueingDeliversEveryEvent ) {
  constexpr int PRODUCERS           = 8;
  constexpr int EVENTS_PER_PRODUCER = 1000;

  auto events   = EventSystem{};
  auto listener = RecordingListener{};
  std::ignore   = events.RegisterListener( &listener );

  {
    auto producers = std::vector<std::jthread>{};
    for ( int p = 0; p < PRODUCERS; ++p ) {
      producers.emplace_back( [&] {
        for ( int i = 0; i < EVENTS_PER_PRODUCER; ++i )
          events.QueueEvent( MakeBroadcast( 1 ) );
      } );
    }

    // Process concurrently with the producers.
    for ( int i = 0; i < 100; ++i )
      events.ProcessEvents();
  }
  events.ProcessEvents();

  EXPECT_EQ( listener.received.size(), static_cast<size_t>( PRODUCERS * EVENTS_PER_PRODUCER ) );
}

TEST( EventSystemTest, UnregisterWaitsForInFlightCallback ) {
  auto events      = EventSystem{};
  auto listener    = RecordingListener{};
  auto started     = std::atomic<bool>{ false };
  auto finished    = std::atomic<bool>{ false };
  listener.onEvent = [&]( skye::EventType ) {
    started = true;
    std::this_thread::sleep_for( std::chrono::milliseconds{ 50 } );
    finished = true;
  };
  const EventSystem::ListenerID id = events.RegisterListener( &listener );
  events.QueueEvent( MakeSingleEvent( 7, id ) );

  auto processor = std::jthread{ [&] { events.ProcessEvents(); } };
  while ( !started )
    std::this_thread::yield();

  events.UnregisterListener( id );
  EXPECT_TRUE( finished );
}

TEST( EventSystemTest, ConcurrentRegistrationWhileProcessing ) {
  auto events = EventSystem{};
  auto done   = std::atomic<bool>{ false };

  auto processor = std::jthread{ [&] {
    while ( !done ) {
      events.QueueEvent( MakeBroadcast( 1 ) );
      events.QueueEvent( MakeChannelEvent( 2, 5 ) );
      events.ProcessEvents();
    }
  } };

  {
    auto workers = std::vector<std::jthread>{};
    for ( int w = 0; w < 4; ++w ) {
      workers.emplace_back( [&] {
        for ( int i = 0; i < 500; ++i ) {
          auto                          listener = RecordingListener{};
          const EventSystem::ListenerID id       = events.RegisterListener( &listener );
          events.RegisterListenerToChannel( id, 5 );
          events.UnregisterListenerFromChannel( id, 5 );
          events.RegisterListenerToChannel( id, 5 );
          // After this returns the listener must never be called again, so destroying it is safe.
          events.UnregisterListener( id );
        }
      } );
    }
  }

  done = true;
}

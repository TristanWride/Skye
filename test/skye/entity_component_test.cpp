#include "skye/Component.hpp"
#include "skye/Entity.hpp"
#include "skye/Util.hpp"

#include <gtest/gtest.h>

#include <iterator>
#include <optional>
#include <ranges>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

namespace {

struct A {};
struct B {};
struct C {};
struct Unregistered {};

using Release = std::pair<skye::EntityID, skye::ComponentID>;

// Registers A, B and C, logging each release callback per type.
class EntityComponentTest : public ::testing::Test {
protected:
  skye::EntityRegistry registry_;
  std::vector<Release> releasedA_;
  std::vector<Release> releasedB_;
  std::vector<Release> releasedC_;

protected:
  explicit EntityComponentTest() {
    registry_.RegisterComponentType<A>( [this]( skye::EntityID entity, skye::ComponentID id ) { releasedA_.emplace_back( entity, id ); } );
    registry_.RegisterComponentType<B>( [this]( skye::EntityID entity, skye::ComponentID id ) { releasedB_.emplace_back( entity, id ); } );
    registry_.RegisterComponentType<C>( [this]( skye::EntityID entity, skye::ComponentID id ) { releasedC_.emplace_back( entity, id ); } );
  }
};

}

static_assert( std::ranges::forward_range<skye::ComponentView<A, B>> );

TEST_F( EntityComponentTest, AddFindHasRemove ) {
  const skye::EntityID entity = registry_.CreateEntity();
  EXPECT_FALSE( registry_.HasComponent<A>( entity ) );
  EXPECT_EQ( registry_.FindComponent<A>( entity ), std::nullopt );

  registry_.AddComponent<A>( entity, 42 );
  EXPECT_TRUE( registry_.HasComponent<A>( entity ) );
  EXPECT_EQ( registry_.FindComponent<A>( entity ), skye::ComponentID{ 42 } );
  EXPECT_FALSE( registry_.HasComponent<B>( entity ) );

  registry_.RemoveComponent<A>( entity );
  EXPECT_FALSE( registry_.HasComponent<A>( entity ) );
  EXPECT_TRUE( releasedA_.empty() );
}

TEST_F( EntityComponentTest, AddingTwiceFails ) {
  const skye::EntityID entity = registry_.CreateEntity();
  registry_.AddComponent<A>( entity, 1 );
  EXPECT_THROW( registry_.AddComponent<A>( entity, 2 ), skye::util::Failure );
  EXPECT_EQ( registry_.FindComponent<A>( entity ), skye::ComponentID{ 1 } );
}

TEST_F( EntityComponentTest, RemovingMissingComponentFails ) {
  const skye::EntityID entity = registry_.CreateEntity();
  EXPECT_THROW( registry_.RemoveComponent<A>( entity ), skye::util::Failure );
}

TEST_F( EntityComponentTest, AddingToDeadEntityFails ) {
  EXPECT_THROW( registry_.AddComponent<A>( 0, 1 ), skye::util::Failure );

  const skye::EntityID entity = registry_.CreateEntity();
  registry_.DestroyEntity( entity );
  EXPECT_THROW( registry_.AddComponent<A>( entity, 1 ), skye::util::Failure );
}

TEST_F( EntityComponentTest, ViewOverOneType ) {
  const skye::EntityID first  = registry_.CreateEntity();
  const skye::EntityID second = registry_.CreateEntity();
  const skye::EntityID third  = registry_.CreateEntity();
  registry_.AddComponent<A>( first, 10 );
  registry_.AddComponent<A>( third, 30 );
  registry_.AddComponent<B>( second, 20 );

  auto seen = std::set<std::pair<skye::EntityID, skye::ComponentID>>{};
  for ( auto [entity, a] : registry_.View<A>() )
    EXPECT_TRUE( seen.emplace( entity, a ).second );

  EXPECT_EQ( seen, ( std::set<std::pair<skye::EntityID, skye::ComponentID>>{ { first, 10 }, { third, 30 } } ) );
}

TEST_F( EntityComponentTest, ViewOverTwoTypes ) {
  auto expected = std::set<std::tuple<skye::EntityID, skye::ComponentID, skye::ComponentID>>{};
  for ( skye::ComponentID i = 0; i < 20; ++i ) {
    const skye::EntityID entity = registry_.CreateEntity();
    if ( i % 2 == 0 ) registry_.AddComponent<A>( entity, 100 + i );
    if ( i % 3 == 0 ) registry_.AddComponent<B>( entity, 200 + i );
    if ( i % 6 == 0 ) expected.emplace( entity, 100 + i, 200 + i );
  }

  auto seen = std::set<std::tuple<skye::EntityID, skye::ComponentID, skye::ComponentID>>{};
  for ( auto [entity, a, b] : registry_.View<A, B>() )
    EXPECT_TRUE( seen.emplace( entity, a, b ).second );
  EXPECT_EQ( seen, expected );

  // Order of the types does not change which entities match.
  auto reversed = std::set<std::tuple<skye::EntityID, skye::ComponentID, skye::ComponentID>>{};
  for ( auto [entity, b, a] : registry_.View<B, A>() )
    reversed.emplace( entity, a, b );
  EXPECT_EQ( reversed, expected );
}

TEST_F( EntityComponentTest, ViewOverThreeTypes ) {
  auto expected = std::set<skye::EntityID>{};
  for ( skye::ComponentID i = 0; i < 30; ++i ) {
    const skye::EntityID entity = registry_.CreateEntity();
    if ( i % 2 == 0 ) registry_.AddComponent<A>( entity, i );
    if ( i % 3 == 0 ) registry_.AddComponent<B>( entity, i );
    if ( i % 5 == 0 ) registry_.AddComponent<C>( entity, i );
    if ( i % 30 == 0 ) expected.insert( entity );
  }

  auto seen = std::set<skye::EntityID>{};
  for ( auto [entity, a, b, c] : registry_.View<A, B, C>() ) {
    EXPECT_EQ( registry_.FindComponent<A>( entity ), a );
    EXPECT_EQ( registry_.FindComponent<B>( entity ), b );
    EXPECT_EQ( registry_.FindComponent<C>( entity ), c );
    EXPECT_TRUE( seen.insert( entity ).second );
  }
  EXPECT_EQ( seen, expected );
}

TEST_F( EntityComponentTest, ViewWithNoMatchesIsEmpty ) {
  const skye::EntityID entity = registry_.CreateEntity();
  registry_.AddComponent<A>( entity, 1 );

  EXPECT_EQ( std::ranges::distance( registry_.View<A, B>() ), 0 );
  EXPECT_EQ( std::ranges::distance( registry_.View<C>() ), 0 );
}

TEST_F( EntityComponentTest, DestroyEntityReleasesEachComponentOnce ) {
  const skye::EntityID entity = registry_.CreateEntity();
  const skye::EntityID other  = registry_.CreateEntity();
  registry_.AddComponent<A>( entity, 1 );
  registry_.AddComponent<C>( entity, 3 );
  registry_.AddComponent<A>( other, 4 );
  registry_.AddComponent<B>( other, 5 );

  registry_.DestroyEntity( entity );

  EXPECT_EQ( releasedA_, ( std::vector<Release>{ { entity, 1 } } ) );
  EXPECT_TRUE( releasedB_.empty() );
  EXPECT_EQ( releasedC_, ( std::vector<Release>{ { entity, 3 } } ) );
  EXPECT_FALSE( registry_.IsAlive( entity ) );
  EXPECT_FALSE( registry_.HasComponent<A>( entity ) );
  EXPECT_FALSE( registry_.HasComponent<C>( entity ) );
  EXPECT_EQ( registry_.FindComponent<A>( other ), skye::ComponentID{ 4 } );
  EXPECT_EQ( registry_.FindComponent<B>( other ), skye::ComponentID{ 5 } );
}

TEST_F( EntityComponentTest, DestroyEntityAfterRemoveDoesNotRelease ) {
  const skye::EntityID entity = registry_.CreateEntity();
  registry_.AddComponent<A>( entity, 1 );
  registry_.RemoveComponent<A>( entity );

  registry_.DestroyEntity( entity );
  EXPECT_TRUE( releasedA_.empty() );
}

TEST_F( EntityComponentTest, DestroyingDeadEntityFailsWithoutReleasing ) {
  const skye::EntityID entity = registry_.CreateEntity();
  registry_.AddComponent<A>( entity, 1 );
  registry_.DestroyEntity( entity );

  EXPECT_THROW( registry_.DestroyEntity( entity ), skye::util::Failure );
  EXPECT_EQ( releasedA_.size(), 1 );
}

TEST_F( EntityComponentTest, StaleEntityHasNoComponents ) {
  const skye::EntityID stale = registry_.CreateEntity();
  registry_.AddComponent<A>( stale, 1 );
  registry_.DestroyEntity( stale );

  const skye::EntityID fresh = registry_.CreateEntity();
  ASSERT_EQ( skye::EntityRegistry::IndexOf( fresh ), skye::EntityRegistry::IndexOf( stale ) );
  registry_.AddComponent<A>( fresh, 2 );

  EXPECT_FALSE( registry_.HasComponent<A>( stale ) );
  EXPECT_EQ( registry_.FindComponent<A>( stale ), std::nullopt );
  EXPECT_THROW( registry_.RemoveComponent<A>( stale ), skye::util::Failure );
  EXPECT_THROW( registry_.AddComponent<A>( stale, 3 ), skye::util::Failure );
  EXPECT_THROW( registry_.DestroyEntity( stale ), skye::util::Failure );
  EXPECT_EQ( registry_.FindComponent<A>( fresh ), skye::ComponentID{ 2 } );
  EXPECT_EQ( releasedA_, ( std::vector<Release>{ { stale, 1 } } ) );
}

TEST_F( EntityComponentTest, UnregisteredTypeFails ) {
  const skye::EntityID entity = registry_.CreateEntity();
  EXPECT_FALSE( registry_.IsComponentTypeRegistered<Unregistered>() );
  EXPECT_THROW( registry_.AddComponent<Unregistered>( entity, 1 ), skye::util::Failure );
  EXPECT_THROW( registry_.RemoveComponent<Unregistered>( entity ), skye::util::Failure );
  EXPECT_THROW( std::ignore = registry_.FindComponent<Unregistered>( entity ), skye::util::Failure );
  EXPECT_THROW( std::ignore = registry_.HasComponent<Unregistered>( entity ), skye::util::Failure );
  EXPECT_THROW( std::ignore = ( registry_.View<A, Unregistered>() ), skye::util::Failure );
  EXPECT_THROW( registry_.UnregisterComponentType<Unregistered>(), skye::util::Failure );
}

TEST_F( EntityComponentTest, DuplicateRegistrationFails ) {
  EXPECT_TRUE( registry_.IsComponentTypeRegistered<A>() );
  EXPECT_THROW( registry_.RegisterComponentType<A>( []( skye::EntityID, skye::ComponentID ) {} ), skye::util::Failure );
}

TEST_F( EntityComponentTest, EmptyReleaseCallbackFails ) {
  EXPECT_THROW( registry_.RegisterComponentType<Unregistered>( nullptr ), skye::util::Failure );
  EXPECT_FALSE( registry_.IsComponentTypeRegistered<Unregistered>() );
}

TEST_F( EntityComponentTest, UnregisterForgetsComponentsWithoutReleasing ) {
  const skye::EntityID entity = registry_.CreateEntity();
  registry_.AddComponent<A>( entity, 1 );

  registry_.UnregisterComponentType<A>();
  EXPECT_FALSE( registry_.IsComponentTypeRegistered<A>() );
  registry_.DestroyEntity( entity );
  EXPECT_TRUE( releasedA_.empty() );

  registry_.RegisterComponentType<A>( []( skye::EntityID, skye::ComponentID ) {} );
  EXPECT_EQ( std::ranges::distance( registry_.View<A>() ), 0 );
}

TEST( EntityRegistryComponentTypesTest, RegistriesAreIndependent ) {
  auto first  = skye::EntityRegistry{};
  auto second = skye::EntityRegistry{};
  first.RegisterComponentType<A>( []( skye::EntityID, skye::ComponentID ) {} );

  EXPECT_TRUE( first.IsComponentTypeRegistered<A>() );
  EXPECT_FALSE( second.IsComponentTypeRegistered<A>() );
  second.RegisterComponentType<A>( []( skye::EntityID, skye::ComponentID ) {} );
}

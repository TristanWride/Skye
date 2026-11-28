#include "skye/Component.hpp"
#include "skye/ComponentStorage.hpp"
#include "skye/Entity.hpp"
#include "skye/Util.hpp"

#include <gtest/gtest.h>

#include <map>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

struct Health {
  int points = 0;
};

// Not trivially copyable, to show systems can store any movable type.
struct Inventory {
  std::string              owner;
  std::vector<std::string> items;
};

// Move-only, to show components need not be copyable.
struct Unique {
  int value = 0;

  explicit Unique( int v )
    : value{ v } {}

  Unique( Unique && )                  = default;
  Unique & operator=( Unique && )      = default;
  Unique( const Unique & )             = delete;
  Unique & operator=( const Unique & ) = delete;
};

}

static_assert( !std::is_copy_constructible_v<skye::ComponentStorage<Health>> );
static_assert( !std::is_move_constructible_v<skye::ComponentStorage<Health>> );
static_assert( std::ranges::forward_range<skye::ComponentStorage<Health>> );
static_assert( std::ranges::forward_range<const skye::ComponentStorage<Health>> );

TEST( ComponentStorageTest, ConstructionRegistersTheType ) {
  auto registry = skye::EntityRegistry{};
  {
    auto storage = skye::ComponentStorage<Health>{ registry };
    EXPECT_TRUE( registry.IsComponentTypeRegistered<Health>() );
    EXPECT_THROW( skye::ComponentStorage<Health>{ registry }, skye::util::Failure );
  }
  EXPECT_FALSE( registry.IsComponentTypeRegistered<Health>() );
}

TEST( ComponentStorageTest, CreateGetDestroy ) {
  auto registry = skye::EntityRegistry{};
  auto storage  = skye::ComponentStorage<Health>{ registry };

  const skye::EntityID    entity = registry.CreateEntity();
  const skye::ComponentID id     = storage.Create( entity, Health{ 10 } );

  EXPECT_TRUE( storage.IsAlive( id ) );
  EXPECT_EQ( registry.FindComponent<Health>( entity ), id );
  EXPECT_EQ( storage.Get( id ).points, 10 );
  EXPECT_EQ( storage.GetForEntity( entity ).points, 10 );
  EXPECT_EQ( storage.Size(), 1 );

  storage.Get( id ).points = 5;
  EXPECT_EQ( std::as_const( storage ).GetForEntity( entity ).points, 5 );

  storage.Destroy( entity );
  EXPECT_FALSE( storage.IsAlive( id ) );
  EXPECT_FALSE( registry.HasComponent<Health>( entity ) );
  EXPECT_THROW( std::ignore = storage.Get( id ), skye::util::Failure );
  EXPECT_THROW( std::ignore = storage.GetForEntity( entity ), skye::util::Failure );
  EXPECT_THROW( storage.Destroy( entity ), skye::util::Failure );
  EXPECT_EQ( storage.Size(), 0 );
  EXPECT_TRUE( registry.IsAlive( entity ) );
}

TEST( ComponentStorageTest, FailedCreateLeavesStorageUnchanged ) {
  auto registry = skye::EntityRegistry{};
  auto storage  = skye::ComponentStorage<Health>{ registry };

  const skye::EntityID    entity = registry.CreateEntity();
  const skye::ComponentID id     = storage.Create( entity, Health{ 1 } );
  EXPECT_THROW( std::ignore = storage.Create( entity, Health{ 2 } ), skye::util::Failure );

  const skye::EntityID dead = registry.CreateEntity();
  registry.DestroyEntity( dead );
  EXPECT_THROW( std::ignore = storage.Create( dead, Health{ 3 } ), skye::util::Failure );

  EXPECT_EQ( storage.Size(), 1 );
  EXPECT_EQ( storage.GetForEntity( entity ).points, 1 );
  EXPECT_EQ( registry.FindComponent<Health>( entity ), id );
}

TEST( ComponentStorageTest, DestroyingEntityFreesItsComponent ) {
  auto registry = skye::EntityRegistry{};
  auto storage  = skye::ComponentStorage<Health>{ registry };

  const skye::EntityID    entity = registry.CreateEntity();
  const skye::EntityID    other  = registry.CreateEntity();
  const skye::ComponentID id     = storage.Create( entity, Health{ 1 } );
  const skye::ComponentID kept   = storage.Create( other, Health{ 2 } );

  registry.DestroyEntity( entity );
  EXPECT_FALSE( storage.IsAlive( id ) );
  EXPECT_THROW( std::ignore = storage.Get( id ), skye::util::Failure );
  EXPECT_EQ( storage.Size(), 1 );
  EXPECT_EQ( storage.Get( kept ).points, 2 );

  // The freed slot is reused at a new generation, so the old ID still does not match.
  const skye::EntityID    newcomer = registry.CreateEntity();
  const skye::ComponentID reused   = storage.Create( newcomer, Health{ 3 } );
  EXPECT_EQ( skye::ComponentIDAllocator::IndexOf( reused ), skye::ComponentIDAllocator::IndexOf( id ) );
  EXPECT_NE( skye::ComponentIDAllocator::GenerationOf( reused ), skye::ComponentIDAllocator::GenerationOf( id ) );
  EXPECT_FALSE( storage.IsAlive( id ) );
  EXPECT_THROW( std::ignore = storage.Get( id ), skye::util::Failure );
  EXPECT_EQ( storage.Get( reused ).points, 3 );
}

TEST( ComponentStorageTest, IterationVisitsLiveComponents ) {
  auto registry = skye::EntityRegistry{};
  auto storage  = skye::ComponentStorage<Health>{ registry };

  auto entities = std::vector<skye::EntityID>{};
  for ( int i = 0; i < 6; ++i ) {
    entities.push_back( registry.CreateEntity() );
    std::ignore = storage.Create( entities.back(), Health{ i } );
  }
  storage.Destroy( entities[1] );
  registry.DestroyEntity( entities[4] );

  for ( auto [entity, id, health] : storage )
    health.points += 100;

  auto seen = std::map<skye::EntityID, int>{};
  for ( const auto [entity, id, health] : std::as_const( storage ) ) {
    EXPECT_EQ( registry.FindComponent<Health>( entity ), id );
    EXPECT_TRUE( seen.emplace( entity, health.points ).second );
  }

  const auto expected = std::map<skye::EntityID, int>{
    { entities[0], 100 }, { entities[2], 102 }, { entities[3], 103 }, { entities[5], 105 }
  };
  EXPECT_EQ( seen, expected );
}

TEST( ComponentStorageTest, StoresNonTriviallyCopyableTypes ) {
  auto registry = skye::EntityRegistry{};
  auto storage  = skye::ComponentStorage<Inventory>{ registry };

  const skye::EntityID    first  = registry.CreateEntity();
  const skye::EntityID    second = registry.CreateEntity();
  const skye::ComponentID id     = storage.Create( first, Inventory{ .owner = "first", .items = { "rope", "lamp" } } );
  std::ignore                    = storage.Create( second, Inventory{ .owner = "second", .items = {} } );

  storage.Get( id ).items.push_back( "key" );
  EXPECT_EQ( storage.GetForEntity( first ).owner, "first" );
  EXPECT_EQ( storage.GetForEntity( first ).items, ( std::vector<std::string>{ "rope", "lamp", "key" } ) );

  registry.DestroyEntity( first );
  const skye::EntityID third = registry.CreateEntity();
  std::ignore                = storage.Create( third, Inventory{ .owner = "third", .items = { "map" } } );

  EXPECT_EQ( storage.GetForEntity( second ).owner, "second" );
  EXPECT_EQ( storage.GetForEntity( third ).items, ( std::vector<std::string>{ "map" } ) );
}

TEST( ComponentStorageTest, StoresMoveOnlyTypes ) {
  auto registry = skye::EntityRegistry{};
  auto storage  = skye::ComponentStorage<Unique>{ registry };

  const skye::EntityID entity = registry.CreateEntity();
  std::ignore                 = storage.Create( entity, Unique{ 7 } );
  EXPECT_EQ( storage.GetForEntity( entity ).value, 7 );
}

TEST( ComponentStorageTest, WorksWithViews ) {
  auto registry = skye::EntityRegistry{};
  auto healths  = skye::ComponentStorage<Health>{ registry };
  auto items    = skye::ComponentStorage<Inventory>{ registry };

  const skye::EntityID both   = registry.CreateEntity();
  const skye::EntityID onlyHP = registry.CreateEntity();
  std::ignore                 = healths.Create( both, Health{ 1 } );
  std::ignore                 = healths.Create( onlyHP, Health{ 2 } );
  std::ignore                 = items.Create( both, Inventory{ .owner = "both", .items = {} } );

  auto visits = 0;
  for ( auto [entity, healthID, inventoryID] : registry.View<Health, Inventory>() ) {
    EXPECT_EQ( entity, both );
    EXPECT_EQ( healths.Get( healthID ).points, 1 );
    EXPECT_EQ( items.Get( inventoryID ).owner, "both" );
    ++visits;
  }
  EXPECT_EQ( visits, 1 );
}

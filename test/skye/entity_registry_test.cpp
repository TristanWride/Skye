#include "skye/Entity.hpp"
#include "skye/Util.hpp"

#include <gtest/gtest.h>

#include <tuple>

TEST( EntityRegistryTest, CanCreateEntity ) {
  auto registry = skye::EntityRegistry{};
  EXPECT_EQ( registry.CreateEntity(), 0 );
  EXPECT_EQ( registry.CreateEntity(), 1 );
}

TEST( EntityRegistryTest, DestroyedIDIsReusedFirst ) {
  auto registry               = skye::EntityRegistry{};
  std::ignore                 = registry.CreateEntity();
  const skye::EntityID second = registry.CreateEntity();
  std::ignore                 = registry.CreateEntity();

  registry.DestroyEntity( second );
  const skye::EntityID recycled = registry.CreateEntity();
  EXPECT_NE( recycled, second );
  EXPECT_EQ( skye::EntityRegistry::IndexOf( recycled ), skye::EntityRegistry::IndexOf( second ) );
  EXPECT_EQ( skye::EntityRegistry::GenerationOf( recycled ), skye::EntityRegistry::GenerationOf( second ) + 1 );
  EXPECT_EQ( registry.CreateEntity(), 3 );
}

TEST( EntityRegistryTest, DestroyingInactiveEntityFails ) {
  auto registry = skye::EntityRegistry{};
  EXPECT_THROW( registry.DestroyEntity( 0 ), skye::util::Failure );

  const skye::EntityID id = registry.CreateEntity();
  registry.DestroyEntity( id );
  EXPECT_THROW( registry.DestroyEntity( id ), skye::util::Failure );
}

TEST( EntityRegistryTest, DestroyingOutOfRangeIDFails ) {
  auto registry = skye::EntityRegistry{};
  EXPECT_THROW( registry.DestroyEntity( skye::EntityRegistry::MAX_ENTITIES - 1 ), skye::util::Failure );
}

TEST( EntityRegistryTest, CreatingPastLimitFails ) {
  auto registry = skye::EntityRegistry{};
  for ( skye::EntityRegistry::size_type i = 0; i < skye::EntityRegistry::MAX_ENTITIES; ++i )
    std::ignore = registry.CreateEntity();

  EXPECT_THROW( std::ignore = registry.CreateEntity(), skye::util::Failure );
}

TEST( EntityRegistryTest, IsAliveTracksLifetime ) {
  auto registry = skye::EntityRegistry{};
  EXPECT_FALSE( registry.IsAlive( 0 ) );

  const skye::EntityID id = registry.CreateEntity();
  EXPECT_TRUE( registry.IsAlive( id ) );

  registry.DestroyEntity( id );
  EXPECT_FALSE( registry.IsAlive( id ) );
}

TEST( EntityRegistryTest, StaleIDIsNotAliveAfterIndexReuse ) {
  auto                 registry = skye::EntityRegistry{};
  const skye::EntityID stale    = registry.CreateEntity();
  registry.DestroyEntity( stale );
  const skye::EntityID fresh = registry.CreateEntity();

  EXPECT_EQ( skye::EntityRegistry::IndexOf( fresh ), skye::EntityRegistry::IndexOf( stale ) );
  EXPECT_TRUE( registry.IsAlive( fresh ) );
  EXPECT_FALSE( registry.IsAlive( stale ) );
}

TEST( EntityRegistryTest, DestroyingStaleIDFails ) {
  auto                 registry = skye::EntityRegistry{};
  const skye::EntityID stale    = registry.CreateEntity();
  registry.DestroyEntity( stale );
  const skye::EntityID fresh = registry.CreateEntity();

  EXPECT_THROW( registry.DestroyEntity( stale ), skye::util::Failure );
  EXPECT_TRUE( registry.IsAlive( fresh ) );
}

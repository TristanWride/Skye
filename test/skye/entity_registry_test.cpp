#include "skye/Entity.hpp"
#include "skye/Util.hpp"
#include "skye/testing/Testing.hpp"

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
  EXPECT_EQ( registry.CreateEntity(), second );
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
  EXPECT_THROW( registry.DestroyEntity( skye::EntityRegistry::MAX_ENTITIES ), skye::util::Failure );
}

TEST( EntityRegistryTest, CreatingPastLimitFails ) {
  auto registry = skye::EntityRegistry{};
  for ( skye::EntityRegistry::size_type i = 0; i < skye::EntityRegistry::MAX_ENTITIES; ++i )
    std::ignore = registry.CreateEntity();

  EXPECT_THROW( std::ignore = registry.CreateEntity(), skye::util::Failure );
}

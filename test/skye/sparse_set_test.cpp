#include "skye/Component.hpp"
#include "skye/EntityID.hpp"
#include "skye/IDAllocator.hpp"
#include "skye/SparseSet.hpp"
#include "skye/Util.hpp"

#include <gtest/gtest.h>

#include <map>
#include <optional>

namespace {

using EntityIDs = skye::IDAllocator<skye::EntityID, skye::ENTITY_GENERATION_BITS>;

}

TEST( SparseSetTest, StartsEmpty ) {
  const auto set = skye::SparseSet{};
  EXPECT_TRUE( set.Empty() );
  EXPECT_EQ( set.Size(), 0 );
  EXPECT_EQ( set.AllocatedPages(), 0 );
  EXPECT_FALSE( set.Contains( 0 ) );
  EXPECT_EQ( set.Find( 0 ), std::nullopt );
}

TEST( SparseSetTest, InsertThenFind ) {
  auto set = skye::SparseSet{};
  set.Insert( 3, 30 );
  set.Insert( 1, 10 );

  EXPECT_EQ( set.Size(), 2 );
  EXPECT_TRUE( set.Contains( 3 ) );
  EXPECT_EQ( set.Find( 3 ), skye::ComponentID{ 30 } );
  EXPECT_EQ( set.Find( 1 ), skye::ComponentID{ 10 } );
  EXPECT_FALSE( set.Contains( 2 ) );
  EXPECT_EQ( set.Find( 2 ), std::nullopt );
}

TEST( SparseSetTest, InsertingTwiceFails ) {
  auto set = skye::SparseSet{};
  set.Insert( 5, 50 );
  EXPECT_THROW( set.Insert( 5, 51 ), skye::util::Failure );
  EXPECT_EQ( set.Find( 5 ), skye::ComponentID{ 50 } );
}

TEST( SparseSetTest, RemoveThenNotFound ) {
  auto set = skye::SparseSet{};
  set.Insert( 7, 70 );
  set.Remove( 7 );

  EXPECT_TRUE( set.Empty() );
  EXPECT_FALSE( set.Contains( 7 ) );
  set.Insert( 7, 71 );
  EXPECT_EQ( set.Find( 7 ), skye::ComponentID{ 71 } );
}

TEST( SparseSetTest, RemovingAbsentEntityFails ) {
  auto set = skye::SparseSet{};
  EXPECT_THROW( set.Remove( 4 ), skye::util::Failure );
  set.Insert( 4, 40 );
  set.Remove( 4 );
  EXPECT_THROW( set.Remove( 4 ), skye::util::Failure );
}

TEST( SparseSetTest, SwapAndPopKeepsOthersFindable ) {
  auto set = skye::SparseSet{};
  for ( skye::EntityID entity = 0; entity < 5; ++entity )
    set.Insert( entity, entity * 10 );

  set.Remove( 1 ); // Moves the last entry (4) into position 1.
  set.Remove( 0 );
  set.Remove( 4 ); // The moved entry.

  EXPECT_EQ( set.Size(), 2 );
  EXPECT_EQ( set.Find( 2 ), skye::ComponentID{ 20 } );
  EXPECT_EQ( set.Find( 3 ), skye::ComponentID{ 30 } );
  EXPECT_FALSE( set.Contains( 0 ) );
  EXPECT_FALSE( set.Contains( 1 ) );
  EXPECT_FALSE( set.Contains( 4 ) );
}

TEST( SparseSetTest, RemovingLastEntryWorks ) {
  auto set = skye::SparseSet{};
  set.Insert( 1, 10 );
  set.Insert( 2, 20 );
  set.Remove( 2 );

  EXPECT_EQ( set.Find( 1 ), skye::ComponentID{ 10 } );
  EXPECT_FALSE( set.Contains( 2 ) );
}

TEST( SparseSetTest, StaleEntityIsNotFound ) {
  const skye::EntityID stale = EntityIDs::MakeID( 9, 0 );
  const skye::EntityID fresh = EntityIDs::MakeID( 9, 1 );

  auto set = skye::SparseSet{};
  set.Insert( fresh, 90 );

  EXPECT_TRUE( set.Contains( fresh ) );
  EXPECT_FALSE( set.Contains( stale ) );
  EXPECT_EQ( set.Find( stale ), std::nullopt );
  EXPECT_THROW( set.Remove( stale ), skye::util::Failure );
  EXPECT_THROW( set.Insert( stale, 91 ), skye::util::Failure );
  EXPECT_EQ( set.Find( fresh ), skye::ComponentID{ 90 } );
}

TEST( SparseSetTest, PagesAreAllocatedLazily ) {
  const skye::EntityID high = EntityIDs::MakeID( static_cast<skye::EntityID>( EntityIDs::INDEX_MASK ), 0 );

  auto set = skye::SparseSet{};
  EXPECT_FALSE( set.Contains( high ) );
  EXPECT_EQ( set.AllocatedPages(), 0 );

  set.Insert( high, 1 );
  EXPECT_EQ( set.AllocatedPages(), 1 );
  EXPECT_EQ( set.Find( high ), skye::ComponentID{ 1 } );

  set.Insert( high - 1, 2 ); // Same page.
  EXPECT_EQ( set.AllocatedPages(), 1 );

  set.Insert( 0, 3 );
  EXPECT_EQ( set.AllocatedPages(), 2 );
}

TEST( SparseSetTest, IterationVisitsEachPairOnce ) {
  auto set = skye::SparseSet{};
  for ( skye::EntityID entity = 0; entity < 10; ++entity )
    set.Insert( entity * 1000, entity + 100 );
  set.Remove( 3000 );
  set.Remove( 0 );

  auto visited = std::map<skye::EntityID, skye::ComponentID>{};
  for ( const auto [entity, componentID] : set )
    EXPECT_TRUE( visited.emplace( entity, componentID ).second );

  ASSERT_EQ( visited.size(), 8 );
  for ( skye::EntityID entity = 1; entity < 10; ++entity )
    if ( entity != 3 )
      EXPECT_EQ( visited.at( entity * 1000 ), entity + 100 );
  EXPECT_EQ( set.Entries().size(), 8 );
}

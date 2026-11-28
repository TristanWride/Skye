#include "skye/IDAllocator.hpp"
#include "skye/Util.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <string_view>
#include <tuple>

TEST( IDAllocatorTest, StartsAtFirstID ) {
  auto ids = skye::IDAllocator<uint32_t>{ 1 };
  EXPECT_EQ( ids.CreateID(), 1u );
  EXPECT_EQ( ids.CreateID(), 2u );
}

TEST( IDAllocatorTest, ReservedIDsCannotBeDestroyed ) {
  auto ids = skye::IDAllocator<uint32_t>{ 1 };
  EXPECT_THROW( ids.DestroyID( 0 ), skye::util::Failure );
}

TEST( IDAllocatorTest, ReusesMostRecentlyDestroyedFirst ) {
  auto           ids = skye::IDAllocator<uint32_t>{};
  const uint32_t a   = ids.CreateID();
  const uint32_t b   = ids.CreateID();
  std::ignore        = ids.CreateID();

  ids.DestroyID( a );
  ids.DestroyID( b );
  EXPECT_EQ( ids.CreateID(), b );
  EXPECT_EQ( ids.CreateID(), a );
  EXPECT_EQ( ids.CreateID(), 3u );
}

TEST( IDAllocatorTest, UnallocatedIDCannotBeDestroyed ) {
  auto ids    = skye::IDAllocator<uint32_t>{};
  std::ignore = ids.CreateID();
  EXPECT_THROW( ids.DestroyID( 5 ), skye::util::Failure );
}

TEST( IDAllocatorTest, FailsWhenFull ) {
  auto ids = skye::IDAllocator<uint8_t>{};
  for ( int i = 0; i < 255; ++i )
    std::ignore = ids.CreateID();

  EXPECT_THROW( std::ignore = ids.CreateID(), skye::util::Failure );
}

TEST( IDAllocatorTest, ErrorsPrintSmallIDsAsNumbers ) {
  auto ids = skye::IDAllocator<uint8_t>{};
  try {
    ids.DestroyID( 65 );
    FAIL() << "DestroyID should have thrown";
  } catch ( const skye::util::Failure & failure ) {
    const std::string_view message = failure.what();
    EXPECT_NE( message.find( "ID 65 has never been allocated." ), std::string_view::npos ) << message;
  }
}

TEST( IDAllocatorTest, RejectsMaxIDsLargerThanType ) {
  EXPECT_THROW( ( skye::IDAllocator<uint8_t>{ 0, 256 } ), skye::util::Failure );
}

TEST( IDAllocatorTest, GenerationalIDsPackIndexAndGeneration ) {
  using Allocator = skye::IDAllocator<uint32_t, 8>;
  static_assert( Allocator::INDEX_BITS == 24 );
  static_assert( Allocator::GENERATION_BITS == 8 );
  static_assert( Allocator::MAX_IDS == ( uint64_t{ 1 } << 24 ) );

  constexpr uint32_t id = Allocator::MakeID( 5, 3 );
  static_assert( id == ( 5u | ( 3u << 24 ) ) );
  static_assert( Allocator::IndexOf( id ) == 5u );
  static_assert( Allocator::GenerationOf( id ) == 3u );

  auto ids = Allocator{ 1 };
  EXPECT_EQ( ids.CreateID(), Allocator::MakeID( 1, 0 ) );
}

TEST( IDAllocatorTest, RecreatedIDReusesIndexWithNextGeneration ) {
  using Allocator    = skye::IDAllocator<uint32_t, 8>;
  auto           ids = Allocator{};
  const uint32_t a   = ids.CreateID();
  ids.DestroyID( a );

  const uint32_t b = ids.CreateID();
  EXPECT_NE( b, a );
  EXPECT_EQ( Allocator::IndexOf( b ), Allocator::IndexOf( a ) );
  EXPECT_EQ( Allocator::GenerationOf( b ), Allocator::GenerationOf( a ) + 1 );
}

TEST( IDAllocatorTest, IsAliveRejectsStaleIDs ) {
  auto           ids = skye::IDAllocator<uint32_t, 8>{ 1 };
  const uint32_t a   = ids.CreateID();
  EXPECT_TRUE( ids.IsAlive( a ) );
  EXPECT_FALSE( ids.IsAlive( 0 ) );
  EXPECT_FALSE( ids.IsAlive( 100 ) );

  ids.DestroyID( a );
  EXPECT_FALSE( ids.IsAlive( a ) );

  const uint32_t b = ids.CreateID();
  EXPECT_TRUE( ids.IsAlive( b ) );
  EXPECT_FALSE( ids.IsAlive( a ) );
}

TEST( IDAllocatorTest, DestroyingStaleIDFails ) {
  auto           ids = skye::IDAllocator<uint32_t, 8>{};
  const uint32_t a   = ids.CreateID();
  ids.DestroyID( a );
  const uint32_t b = ids.CreateID();

  EXPECT_THROW( ids.DestroyID( a ), skye::util::Failure );
  EXPECT_TRUE( ids.IsAlive( b ) );
}

TEST( IDAllocatorTest, GenerationWrapsAfterAllGenerationsUsed ) {
  using Allocator   = skye::IDAllocator<uint8_t, 2>;
  auto          ids = Allocator{};
  const uint8_t a   = ids.CreateID();
  EXPECT_EQ( Allocator::GenerationOf( a ), 0u );

  uint8_t id = a;
  for ( int generation = 1; generation < 4; ++generation ) {
    ids.DestroyID( id );
    id = ids.CreateID();
    EXPECT_EQ( Allocator::IndexOf( id ), Allocator::IndexOf( a ) );
    EXPECT_EQ( Allocator::GenerationOf( id ), generation );
  }

  ids.DestroyID( id );
  EXPECT_EQ( ids.CreateID(), a );
}

TEST( IDAllocatorTest, GenerationalErrorsPrintIndexAndGeneration ) {
  using Allocator = skye::IDAllocator<uint8_t, 2>;
  auto ids        = Allocator{};
  try {
    ids.DestroyID( Allocator::MakeID( 5, 2 ) );
    FAIL() << "DestroyID should have thrown";
  } catch ( const skye::util::Failure & failure ) {
    const std::string_view message = failure.what();
    EXPECT_NE( message.find( "ID 133 (index 5, generation 2) has never been allocated." ), std::string_view::npos ) << message;
  }
}

TEST( IDAllocatorTest, GenerationalAllocatorUsesEveryIndex ) {
  auto ids = skye::IDAllocator<uint8_t, 2>{};
  for ( int i = 0; i < 64; ++i )
    std::ignore = ids.CreateID();

  EXPECT_THROW( std::ignore = ids.CreateID(), skye::util::Failure );
  EXPECT_THROW( ( skye::IDAllocator<uint8_t, 2>{ 0, 65 } ), skye::util::Failure );
}

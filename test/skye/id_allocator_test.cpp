#include "skye/IDAllocator.hpp"
#include "skye/Util.hpp"
#include "skye/testing/Testing.hpp"

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

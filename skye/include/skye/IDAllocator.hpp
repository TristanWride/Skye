#pragma once

#include "skye/Util.hpp"

#include <concepts>
#include <cstdint>
#include <format>
#include <limits>
#include <string>
#include <vector>

namespace skye {

// Hands out IDs whose index is in [firstID, maxIDs), reusing the most recently destroyed index first.
// With GenerationBits > 0, an ID packs [ generation : GenerationBits ][ index : INDEX_BITS ], generation in the high
// bits. Destroying an ID bumps its slot's generation, so a stale copy of it no longer matches once the index is
// reused: IsAlive() returns false for it and DestroyID() rejects it. The generation wraps to 0 after
// 2^GenerationBits reuses of one slot, so a very old stale ID can in theory match again.
// With GenerationBits == 0, IDs are plain indices and destroyed IDs are reused unchanged.
// Not thread-safe: callers that share one across threads must synchronise access themselves.
template <std::unsigned_integral IntType, unsigned GenerationBits = 0>
class IDAllocator {
public:
  using size_type = uint64_t;

  struct Entry {
    bool    isActive   = false;
    IntType generation = 0;
    IntType nextFreeID = 0; // An index, not a full ID.
  };

  static constexpr unsigned TYPE_BITS       = std::numeric_limits<IntType>::digits;
  static constexpr unsigned GENERATION_BITS = GenerationBits;
  static constexpr unsigned INDEX_BITS      = TYPE_BITS - GENERATION_BITS;

  static_assert( GENERATION_BITS < TYPE_BITS, "GenerationBits must leave at least one index bit." );

  static constexpr IntType INDEX_MASK = [] {
    if constexpr ( GENERATION_BITS == 0 )
      return std::numeric_limits<IntType>::max();
    else
      return static_cast<IntType>( ( IntType{ 1 } << INDEX_BITS ) - 1 );
  }();

  static constexpr IntType GENERATION_MASK = static_cast<IntType>( ( IntType{ 1 } << GENERATION_BITS ) - 1 );

  // Largest allowed maxIDs. Every index, and the next-free value after the last one, must fit in IntType. Without
  // generations that caps it at the type's maximum; with them the index bits are the limit, and all 2^INDEX_BITS
  // indices are usable because the next-free value is stored in a full IntType.
  static constexpr size_type MAX_IDS = [] {
    if constexpr ( GENERATION_BITS == 0 )
      return size_type{ std::numeric_limits<IntType>::max() };
    else
      return size_type{ 1 } << INDEX_BITS;
  }();

private:
  std::vector<Entry> entries_;
  IntType            firstFreeID_; // An index, not a full ID.
  size_type          maxIDs_;
  size_type          activeIDs_{ 0 };

public:
  // IDs below firstID are reserved and never handed out (e.g. 0 as a null ID).
  explicit IDAllocator( IntType firstID = 0, size_type maxIDs = MAX_IDS )
    : entries_( firstID ) // Parentheses: size constructor, not an initializer list.
    , firstFreeID_{ firstID }
    , maxIDs_{ maxIDs } {
    if ( maxIDs_ > MAX_IDS )
      util::Fail( "Maximum number of IDs ({}) exceeds the maximum allowed ({}) for this type.", maxIDs_, MAX_IDS );
  }

  // Returns the index part of an ID.
  [[nodiscard]] static constexpr IntType IndexOf( IntType id ) noexcept {
    return static_cast<IntType>( id & INDEX_MASK );
  }

  // Returns the generation part of an ID (always 0 without generations).
  [[nodiscard]] static constexpr IntType GenerationOf( IntType id ) noexcept {
    if constexpr ( GENERATION_BITS == 0 )
      return 0;
    else
      return static_cast<IntType>( id >> INDEX_BITS );
  }

  // Packs an index and a generation into an ID. Bits outside INDEX_MASK and GENERATION_MASK are dropped.
  [[nodiscard]] static constexpr IntType MakeID( IntType index, IntType generation ) noexcept {
    if constexpr ( GENERATION_BITS == 0 )
      return index;
    else
      return static_cast<IntType>( ( index & INDEX_MASK ) | ( ( generation & GENERATION_MASK ) << INDEX_BITS ) );
  }

  // Returns a new ID, reusing the most recently destroyed index first (at its next generation).
  [[nodiscard]] IntType CreateID() {
    const IntType index = firstFreeID_;
    if ( index >= maxIDs_ )
      util::Fail( "Maximum number of IDs ({}) reached.", maxIDs_ );

    // The free list always ends at entries_.size(), so growing by one entry is enough.
    if ( index == entries_.size() )
      entries_.push_back( Entry{ .isActive = false, .generation = 0, .nextFreeID = static_cast<IntType>( index + 1 ) } );

    Entry & entry  = entries_[index];
    firstFreeID_   = entry.nextFreeID;
    entry.isActive = true;
    ++activeIDs_;
    return MakeID( index, entry.generation );
  }

  // Frees an active ID's index for reuse. Fails if the ID was never allocated, is not active, or is stale.
  void DestroyID( IntType id ) {
    const IntType index = IndexOf( id );
    if ( index >= entries_.size() )
      util::Fail( "{} has never been allocated.", Describe( id ) );

    Entry & entry = entries_[index];
    if ( !entry.isActive )
      util::Fail( "{} is not active and cannot be destroyed.", Describe( id ) );

    if ( entry.generation != GenerationOf( id ) )
      util::Fail( "{} is stale: its slot is now at generation {:d}.", Describe( id ), entry.generation );

    entry.isActive   = false;
    entry.generation = static_cast<IntType>( ( entry.generation + 1 ) & GENERATION_MASK );
    entry.nextFreeID = firstFreeID_;
    firstFreeID_     = index;
    --activeIDs_;
  }

  // Returns whether the ID is currently allocated and, with generations, not stale.
  [[nodiscard]] bool IsAlive( IntType id ) const noexcept {
    const IntType index = IndexOf( id );
    if ( index >= entries_.size() ) return false;

    const Entry & entry = entries_[index];
    return entry.isActive && entry.generation == GenerationOf( id );
  }

private:
  // Formats an ID for error messages, adding its index and generation when generations are in use.
  [[nodiscard]] static std::string Describe( IntType id ) {
    if constexpr ( GENERATION_BITS == 0 )
      return std::format( "ID {:d}", id );
    else
      return std::format( "ID {:d} (index {:d}, generation {:d})", id, IndexOf( id ), GenerationOf( id ) );
  }
};

}

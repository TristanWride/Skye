#pragma once

#include "skye/Util.hpp"

#include <concepts>
#include <cstdint>
#include <limits>
#include <mutex>
#include <vector>

namespace skye {

// Hands out IDs in [firstID, maxIDs), reusing the most recently destroyed ID first. Thread-safe.
template <std::unsigned_integral IntType>
class IDAllocator {
public:
  using size_type = uint64_t;

  struct Entry {
    bool    isActive   = false;
    IntType nextFreeID = 0;
  };

  // Largest allowed maxIDs: every ID, and the next-free value after the last one, must fit in IntType.
  static constexpr size_type MAX_IDS = std::numeric_limits<IntType>::max();

private:
  std::mutex         mutex_;
  std::vector<Entry> entries_;
  IntType            firstFreeID_;
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

  // Returns a new ID, reusing the most recently destroyed ID first.
  [[nodiscard]] IntType CreateID() {
    auto lock = std::lock_guard{ mutex_ };

    const IntType newID = firstFreeID_;
    if ( newID >= maxIDs_ )
      util::Fail( "Maximum number of IDs ({}) reached.", maxIDs_ );

    // The free list always ends at entries_.size(), so growing by one entry is enough.
    if ( newID == entries_.size() )
      entries_.push_back( Entry{ .isActive = false, .nextFreeID = static_cast<IntType>( newID + 1 ) } );

    Entry & entry  = entries_[newID];
    firstFreeID_   = entry.nextFreeID;
    entry.isActive = true;
    ++activeIDs_;
    return newID;
  }

  // Frees an active ID for reuse.
  void DestroyID( IntType id ) {
    auto lock = std::lock_guard{ mutex_ };

    if ( id >= entries_.size() )
      util::Fail( "ID {} has never been allocated.", id );

    Entry & entry = entries_[id];
    if ( !entry.isActive )
      util::Fail( "ID {} is not active and cannot be destroyed.", id );

    entry.isActive   = false;
    entry.nextFreeID = firstFreeID_;
    firstFreeID_     = id;
    --activeIDs_;
  }
};

}

#pragma once

#include "skye/Component.hpp"
#include "skye/EntityID.hpp"
#include "skye/IDAllocator.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace skye {

// Maps entities to the ComponentID of their component of one type.
// A paged sparse array, indexed by entity index, holds each entity's position in a dense array of (EntityID,
// ComponentID) entries. Pages are allocated on first use, so high entity indices cost one page, not the whole range.
// The dense array stores full EntityIDs, so a stale entity whose index was reused is never found.
// Removal swaps the last entry into the removed one's place, so dense order is not stable.
// Not thread-safe.
class SparseSet {
public:
  using size_type = uint64_t;

  struct Entry {
    EntityID    entity      = 0;
    ComponentID componentID = 0;
  };

  using const_iterator = std::vector<Entry>::const_iterator;

  static constexpr size_type PAGE_SIZE = 4096;

private:
  using EntityIDs = IDAllocator<EntityID, ENTITY_GENERATION_BITS>;
  using Position  = uint32_t;
  using Page      = std::array<Position, PAGE_SIZE>;

  static constexpr Position NO_POSITION = std::numeric_limits<Position>::max();

private:
  std::vector<std::unique_ptr<Page>> pages_;
  std::vector<Entry>                 dense_;

public:
  // Starts empty, with no pages allocated.
  explicit SparseSet() = default;

  // Records the entity's ComponentID. Fails if the entity, or a stale entity with the same index, is already present.
  void Insert( EntityID entity, ComponentID componentID );

  // Removes the entity, moving the last entry into its place. Fails if the entity is not present.
  void Remove( EntityID entity );

  // Returns whether the entity is present (false for a stale entity whose index is held by another generation).
  [[nodiscard]] bool Contains( EntityID entity ) const noexcept;

  // Returns the entity's ComponentID, or nothing if the entity is not present.
  [[nodiscard]] std::optional<ComponentID> Find( EntityID entity ) const noexcept;

  // Returns the number of entities present.
  [[nodiscard]] size_type Size() const noexcept;

  // Returns whether no entities are present.
  [[nodiscard]] bool Empty() const noexcept;

  // Returns the number of sparse pages allocated so far.
  [[nodiscard]] size_type AllocatedPages() const noexcept;

  // Returns every (EntityID, ComponentID) entry, in dense order.
  [[nodiscard]] std::span<const Entry> Entries() const noexcept;

  // Iterates over every (EntityID, ComponentID) entry, in dense order.
  [[nodiscard]] const_iterator begin() const noexcept;

  // End of the iteration started by begin().
  [[nodiscard]] const_iterator end() const noexcept;

private:
  // Returns the dense position stored for the entity's index, or NO_POSITION. Ignores the generation.
  [[nodiscard]] Position PositionOf( EntityID entity ) const noexcept;

  // Returns the sparse slot for the entity's index, allocating its page if needed.
  [[nodiscard]] Position & SlotFor( EntityID entity );
};

}

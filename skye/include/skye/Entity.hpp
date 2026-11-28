#pragma once

#include "skye/IDAllocator.hpp"

#include <cstddef>
#include <cstdint>

namespace skye {

using EntityID = uint32_t;

class EntityRegistry {
public:
  using size_type = uint64_t;
  // 14 generation bits leave 18 index bits: exactly MAX_ENTITIES indices.
  using EntityIDAllocator = IDAllocator<EntityID, 14>;

  static constexpr size_type MAX_ENTITIES = 1 << 18;
  // Upper bound: IDAllocator grows its storage on demand.
  static constexpr size_t ENTITY_MEM = MAX_ENTITIES * sizeof( EntityIDAllocator::Entry );

  static_assert( MAX_ENTITIES <= EntityIDAllocator::MAX_IDS );

private:
  EntityIDAllocator entityIDs_{ 0, MAX_ENTITIES };

public:
  // Starts with no entities.
  explicit EntityRegistry() = default;

  // Returns the index part of an entity ID, stable for the entity's lifetime and reused after it is destroyed.
  [[nodiscard]] static constexpr EntityID IndexOf( EntityID entityID ) noexcept {
    return EntityIDAllocator::IndexOf( entityID );
  }

  // Returns the generation part of an entity ID.
  [[nodiscard]] static constexpr EntityID GenerationOf( EntityID entityID ) noexcept {
    return EntityIDAllocator::GenerationOf( entityID );
  }

  // Returns a new entity ID, reusing the most recently destroyed index first with a new generation.
  [[nodiscard]] EntityID CreateEntity();

  // Frees a live entity's index for reuse. Fails if the ID is stale or not alive.
  void DestroyEntity( EntityID entityID );

  // Returns whether the entity ID refers to a live entity (false for stale IDs whose index was reused).
  [[nodiscard]] bool IsAlive( EntityID entityID ) const noexcept;
};

}

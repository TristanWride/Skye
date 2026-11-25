#pragma once

#include "skye/IDAllocator.hpp"

#include <cstddef>
#include <cstdint>

namespace skye {

using EntityID = uint32_t;

class EntityRegistry {
public:
  using size_type = uint64_t;

  static constexpr size_type MAX_ENTITIES = 1 << 18;
  // Upper bound: IDAllocator grows its storage on demand.
  static constexpr size_t ENTITY_MEM = MAX_ENTITIES * sizeof( IDAllocator<EntityID>::Entry );

private:
  IDAllocator<EntityID> entityIDs_{ 0, MAX_ENTITIES };

public:
  // Starts with no entities.
  explicit EntityRegistry() = default;

  // Returns a new entity ID, reusing the most recently destroyed ID first.
  [[nodiscard]] EntityID CreateEntity();

  // Frees an active entity's ID for reuse.
  void DestroyEntity( EntityID entityID );
};

}

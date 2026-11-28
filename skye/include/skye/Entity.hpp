#pragma once

#include "skye/Component.hpp"
#include "skye/ComponentView.hpp"
#include "skye/EntityID.hpp"
#include "skye/IDAllocator.hpp"
#include "skye/SparseSet.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace skye {

// Hands out entity IDs and records which components each entity has.
// The registry stores only ComponentIDs, never component data: systems own the data, create components, and record
// them here with AddComponent(). For each registered component type, a SparseSet maps entities to ComponentIDs.
// Destroying an entity calls the release callback of every component type it has, so the owning systems free their
// slots, before the entity ID is freed.
// Modifying the registry (creating or destroying entities, adding or removing components, registering or
// unregistering types) while iterating a View() is not allowed.
// Not thread-safe.
class EntityRegistry {
public:
  using size_type = uint64_t;
  // ENTITY_GENERATION_BITS leaves exactly MAX_ENTITIES indices.
  using EntityIDAllocator = IDAllocator<EntityID, ENTITY_GENERATION_BITS>;
  // Frees the owning system's slot for an entity's component. Must not modify the registry.
  using ReleaseCallback = std::function<void( EntityID, ComponentID )>;

  static constexpr size_type MAX_ENTITIES = 1 << 18;
  // Upper bound: IDAllocator grows its storage on demand.
  static constexpr size_t ENTITY_MEM = MAX_ENTITIES * sizeof( EntityIDAllocator::Entry );

  static_assert( MAX_ENTITIES <= EntityIDAllocator::MAX_IDS );

private:
  struct ComponentType {
    SparseSet       set;
    ReleaseCallback release;
  };

private:
  EntityIDAllocator entityIDs_{ 0, MAX_ENTITIES };
  // Indexed by ComponentTypeID; null for types not registered with this registry. unique_ptr keeps each set at a
  // fixed address while other types are registered.
  std::vector<std::unique_ptr<ComponentType>> componentTypes_;

public:
  // Starts with no entities and no component types.
  explicit EntityRegistry() = default;

  // Neither copyable nor movable: ComponentStorage and release callbacks refer to the registry by address.
  EntityRegistry( const EntityRegistry & )             = delete;
  EntityRegistry & operator=( const EntityRegistry & ) = delete;
  EntityRegistry( EntityRegistry && )                  = delete;
  EntityRegistry & operator=( EntityRegistry && )      = delete;

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

  // Releases each of the entity's components through its type's release callback, then frees the entity's index for
  // reuse. Fails if the ID is stale or not alive.
  void DestroyEntity( EntityID entityID );

  // Returns whether the entity ID refers to a live entity (false for stale IDs whose index was reused).
  [[nodiscard]] bool IsAlive( EntityID entityID ) const noexcept;

  // Registers T as a component type, with the callback DestroyEntity() uses to free T components. Fails if T is
  // already registered or the callback is empty.
  template <class T>
  void RegisterComponentType( ReleaseCallback release ) {
    RegisterComponentType( ComponentTypeOf<T>(), std::move( release ) );
  }

  // Unregisters T, forgetting its components without calling the release callback. T can then be registered again.
  // Fails if T is not registered.
  template <class T>
  void UnregisterComponentType() {
    UnregisterComponentType( ComponentTypeOf<T>() );
  }

  // Returns whether T is registered as a component type.
  template <class T>
  [[nodiscard]] bool IsComponentTypeRegistered() const noexcept {
    return IsComponentTypeRegistered( ComponentTypeOf<T>() );
  }

  // Records the entity's T component. Fails if T is not registered, the entity is not alive, or it already has a T.
  template <class T>
  void AddComponent( EntityID entityID, ComponentID componentID ) {
    AddComponent( ComponentTypeOf<T>(), entityID, componentID );
  }

  // Forgets the entity's T component without calling the release callback: the caller frees the data. Fails if T is
  // not registered or the entity has no T.
  template <class T>
  void RemoveComponent( EntityID entityID ) {
    SetOf( ComponentTypeOf<T>() ).Remove( entityID );
  }

  // Returns the ComponentID of the entity's T component, or nothing if it has none. Fails if T is not registered.
  template <class T>
  [[nodiscard]] std::optional<ComponentID> FindComponent( EntityID entityID ) const {
    return SetOf( ComponentTypeOf<T>() ).Find( entityID );
  }

  // Returns whether the entity has a T component. Fails if T is not registered.
  template <class T>
  [[nodiscard]] bool HasComponent( EntityID entityID ) const {
    return SetOf( ComponentTypeOf<T>() ).Contains( entityID );
  }

  // Returns a range of ( EntityID, ComponentID... ) for every entity that has all of Ts. Fails if any of Ts is not
  // registered. The registry must not be modified while the view is iterated, and the view must not outlive it.
  template <class... Ts>
  [[nodiscard]] ComponentView<Ts...> View() const {
    return ComponentView<Ts...>{ typename ComponentView<Ts...>::Sets{ &SetOf( ComponentTypeOf<Ts>() )... } };
  }

private:
  // Registers a component type by ID. Fails as RegisterComponentType<T>() does.
  void RegisterComponentType( ComponentTypeID type, ReleaseCallback release );

  // Unregisters a component type by ID. Fails if it is not registered.
  void UnregisterComponentType( ComponentTypeID type );

  // Returns whether a component type is registered, by ID.
  [[nodiscard]] bool IsComponentTypeRegistered( ComponentTypeID type ) const noexcept;

  // Records a component by type ID. Fails as AddComponent<T>() does.
  void AddComponent( ComponentTypeID type, EntityID entityID, ComponentID componentID );

  // Returns a registered type's set. Fails if the type is not registered.
  [[nodiscard]] SparseSet & SetOf( ComponentTypeID type );

  // Returns a registered type's set. Fails if the type is not registered.
  [[nodiscard]] const SparseSet & SetOf( ComponentTypeID type ) const;
};

}

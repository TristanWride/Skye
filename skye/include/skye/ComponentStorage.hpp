#pragma once

#include "skye/Component.hpp"
#include "skye/Entity.hpp"
#include "skye/EntityID.hpp"
#include "skye/Util.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace skye {

// Building block for systems: owns every T component and its ComponentID, and keeps the registry in step.
// T can be any movable type. Components live in slots indexed by ComponentID index; a destroyed component's slot is
// reused at a new generation, so stale ComponentIDs are rejected.
// Registers T with the registry on construction, with a release callback that frees T's slot when an entity is
// destroyed, and unregisters T on destruction (forgetting any remaining T components). So T can have only one
// storage per registry at a time, and the registry must outlive the storage.
// Neither copyable nor movable: the registry's callback points at this storage.
// Not thread-safe.
template <class T>
class ComponentStorage {
public:
  using size_type = uint64_t;

  static_assert( std::is_move_constructible_v<T>, "Components must be movable." );

private:
  struct Slot {
    EntityID         entity      = 0;
    ComponentID      componentID = 0;
    std::optional<T> value;
  };

  using Slots = std::vector<Slot>;

public:
  // Iterates over live components as ( EntityID, ComponentID, T & ) tuples, const T & for a const storage.
  template <bool IsConst>
  class BasicIterator {
  public:
    using Value            = std::conditional_t<IsConst, const T, T>;
    using value_type       = std::tuple<EntityID, ComponentID, Value &>;
    using reference        = value_type;
    using difference_type  = std::ptrdiff_t;
    using iterator_concept = std::forward_iterator_tag;

  private:
    using SlotsPointer = std::conditional_t<IsConst, const Slots *, Slots *>;

  private:
    SlotsPointer slots_{ nullptr };
    size_t       position_{ 0 };

  public:
    // Iterator of no storage, as forward iterators require.
    explicit BasicIterator() = default;

    // Starts at the first live slot at or after position.
    explicit BasicIterator( SlotsPointer slots, size_t position ) noexcept
      : slots_{ slots }
      , position_{ position } {
      SkipEmpty();
    }

    // Returns ( EntityID, ComponentID, T & ) for the current component.
    [[nodiscard]] reference operator*() const noexcept {
      auto & slot = ( *slots_ )[position_];
      return reference{ slot.entity, slot.componentID, *slot.value };
    }

    // Moves to the next live component.
    BasicIterator & operator++() noexcept {
      ++position_;
      SkipEmpty();
      return *this;
    }

    // Moves to the next live component, returning the previous position.
    BasicIterator operator++( int ) noexcept {
      BasicIterator previous = *this;
      ++*this;
      return previous;
    }

    // Returns whether two iterators of the same storage are at the same slot.
    [[nodiscard]] bool operator==( const BasicIterator & other ) const noexcept {
      return position_ == other.position_;
    }

  private:
    // Advances to the next slot holding a component, or the end.
    void SkipEmpty() noexcept {
      if ( slots_ == nullptr ) return;

      while ( position_ < slots_->size() && !( *slots_ )[position_].value )
        ++position_;
    }
  };

  using iterator       = BasicIterator<false>;
  using const_iterator = BasicIterator<true>;

private:
  EntityRegistry &     registry_;
  ComponentIDAllocator componentIDs_;
  Slots                slots_;
  size_type            size_{ 0 };

public:
  // Registers T with the registry. Fails if T is already registered there.
  explicit ComponentStorage( EntityRegistry & registry )
    : registry_{ registry } {
    registry_.RegisterComponentType<T>( [this]( EntityID, ComponentID componentID ) { Release( componentID ); } );
  }

  ComponentStorage( const ComponentStorage & )             = delete;
  ComponentStorage & operator=( const ComponentStorage & ) = delete;

  // Unregisters T from the registry, which forgets any remaining T components without releasing them.
  ~ComponentStorage() {
    // Skipped if someone else already unregistered T, rather than failing in a destructor.
    if ( registry_.IsComponentTypeRegistered<T>() )
      registry_.UnregisterComponentType<T>();
  }

  // Stores value as the entity's T component and returns its new ComponentID. Fails if the entity is not alive or
  // already has a T, leaving the storage unchanged.
  [[nodiscard]] ComponentID Create( EntityID entity, T value ) {
    const ComponentID componentID = componentIDs_.CreateID();
    try {
      const size_t index = ComponentIDAllocator::IndexOf( componentID );
      if ( index >= slots_.size() )
        slots_.resize( index + 1 );

      Slot & slot      = slots_[index];
      slot.entity      = entity;
      slot.componentID = componentID;
      slot.value.emplace( std::move( value ) );
      ++size_;
      registry_.AddComponent<T>( entity, componentID );
    } catch ( ... ) {
      Release( componentID );
      throw;
    }

    return componentID;
  }

  // Destroys the entity's T component. Fails if the entity has no T.
  void Destroy( EntityID entity ) {
    const std::optional<ComponentID> componentID = registry_.FindComponent<T>( entity );
    if ( !componentID )
      util::Fail( "Entity {:d} has no component to destroy.", entity );

    registry_.RemoveComponent<T>( entity );
    Release( *componentID );
  }

  // Returns whether the ComponentID refers to a live component of this storage (false for stale IDs).
  [[nodiscard]] bool IsAlive( ComponentID componentID ) const noexcept {
    return componentIDs_.IsAlive( componentID );
  }

  // Returns the component with this ComponentID. Fails if the ID is stale or unknown.
  [[nodiscard]] T & Get( ComponentID componentID ) {
    return const_cast<T &>( std::as_const( *this ).Get( componentID ) );
  }

  // Returns the component with this ComponentID. Fails if the ID is stale or unknown.
  [[nodiscard]] const T & Get( ComponentID componentID ) const {
    if ( !IsAlive( componentID ) )
      util::Fail( "Component ID {:d} is stale or unknown.", componentID );

    return *slots_[ComponentIDAllocator::IndexOf( componentID )].value;
  }

  // Returns the entity's T component. Fails if the entity has no T.
  [[nodiscard]] T & GetForEntity( EntityID entity ) {
    return const_cast<T &>( std::as_const( *this ).GetForEntity( entity ) );
  }

  // Returns the entity's T component. Fails if the entity has no T.
  [[nodiscard]] const T & GetForEntity( EntityID entity ) const {
    const std::optional<ComponentID> componentID = registry_.FindComponent<T>( entity );
    if ( !componentID )
      util::Fail( "Entity {:d} has no component of this type.", entity );

    return Get( *componentID );
  }

  // Returns the number of live components.
  [[nodiscard]] size_type Size() const noexcept {
    return size_;
  }

  // Iterates over live components as ( EntityID, ComponentID, T & ), in slot order.
  [[nodiscard]] iterator begin() noexcept {
    return iterator{ &slots_, 0 };
  }

  // End of the iteration started by begin().
  [[nodiscard]] iterator end() noexcept {
    return iterator{ &slots_, slots_.size() };
  }

  // Iterates over live components as ( EntityID, ComponentID, const T & ), in slot order.
  [[nodiscard]] const_iterator begin() const noexcept {
    return const_iterator{ &slots_, 0 };
  }

  // End of the iteration started by begin().
  [[nodiscard]] const_iterator end() const noexcept {
    return const_iterator{ &slots_, slots_.size() };
  }

private:
  // Destroys the component's value, if any, and frees its ComponentID. Leaves the registry alone. Fails if the ID is
  // stale or unknown.
  void Release( ComponentID componentID ) {
    if ( !IsAlive( componentID ) )
      util::Fail( "Component ID {:d} is stale or unknown and cannot be released.", componentID );

    const size_t index = ComponentIDAllocator::IndexOf( componentID );
    if ( index < slots_.size() && slots_[index].value ) {
      slots_[index].value.reset();
      --size_;
    }

    componentIDs_.DestroyID( componentID );
  }
};

}

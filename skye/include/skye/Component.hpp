#pragma once

#include "skye/IDAllocator.hpp"

#include <cstdint>
#include <type_traits>

namespace skye {

// Handle to one component, issued by the system that owns the component's data.
using ComponentID = uint32_t;

// 8 generation bits leave 24 index bits, so a stale ComponentID does not match its slot once it is reused.
using ComponentIDAllocator = IDAllocator<ComponentID, 8>;

// Small number identifying a component type, used to index per-type data in EntityRegistry.
using ComponentTypeID = uint32_t;

namespace detail {

// Returns the next unused component type ID. Defined in Component.cpp so every type shares one counter.
[[nodiscard]] ComponentTypeID NextComponentTypeID() noexcept;

template <class T>
[[nodiscard]] ComponentTypeID ComponentTypeOfUnqualified() noexcept {
  static const ComponentTypeID id = NextComponentTypeID();
  return id;
}

}

// Returns T's component type ID, ignoring const, volatile and references. IDs are assigned at runtime, in the order
// types are first used, so they are only stable within one process: never save or send them.
template <class T>
[[nodiscard]] ComponentTypeID ComponentTypeOf() noexcept {
  return detail::ComponentTypeOfUnqualified<std::remove_cvref_t<T>>();
}

}

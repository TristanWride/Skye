#pragma once

#include <concepts>
#include <cstdint>
#include <type_traits>

using EntityId = std::uint32_t;

template <typename CompM>
concept ComponentManager = requires {
    typename CompM::ComponentType;
};

template <typename CompM, typename Comp>
concept ComponentManagerFor = ComponentManager<CompM> 
    && std::is_same_v<typename CompM::ComponentType, Comp> 
    && requires (CompM m, EntityId id) {
        { m.Get(id) } -> std::convertible_to<Comp>;
};

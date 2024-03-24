#pragma once

#include <concepts>
#include <cstdint>

using EntityId = std::uint32_t;

template <typename CompM>
concept ComponentManager = requires {
    typename CompM::ComponentType;
};

template <typename CompM, typename Comp>
concept ComponentManagerFor = ComponentManager<CompM> 
    && std::same_as<typename CompM::ComponentType, Comp>;

template <typename ECS, typename Comp>
concept SupportsComponent = ECS::template HasComponentManager<Comp>();

template <typename ECS, typename... Comps>
concept SupportsComponents = (SupportsComponent<ECS, Comps> && ...);

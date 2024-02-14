#pragma once

#include "debugutils.h"
#include "opaquetypes.h"

#include "meshcomponent.h"

#include <cstdint>
#include <utility>
#include <unordered_map>
#include <vector>
#include <ranges>
#include <type_traits>

template <typename ComponentType> 
struct FindComponentManager;

template <>
struct FindComponentManager<MeshComponent> {
    using ManagerType = MeshComponentManager;
};

template <typename ComponentManagerType>
concept ComponentManager = requires {
     typename ComponentManagerType::ComponentType;
} && std::is_same_v<
    typename FindComponentManager<typename ComponentManagerType::ComponentType>::ManagerType, 
    ComponentManagerType
> && requires (ComponentManagerType cm) {
    std::ranges::range<decltype(cm.AllEntitiesAndComponents())>;
    { cm.AllEntitiesAndComponents().begin()->first } -> std::convertible_to<EntityId>;
    { cm.AllEntitiesAndComponents().begin()->second } -> std::convertible_to<typename ComponentManagerType::ComponentType>;
} && requires (ComponentManagerType cm, EntityId entityId) {
    { cm.GetComponent(entityId) } -> std::convertible_to<typename ComponentManagerType::ComponentType>;
};

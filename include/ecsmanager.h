#pragma once

#include "debugutils.h"

#include <concepts>
#include <cstdint>
#include <unordered_map>
#include <vector>

using ComponentId = std::uint32_t;
using EntityId = std::uint32_t;

template <typename ComponentType>
struct ComponentManager {
    std::unordered_map<EntityId, ComponentId> entityIdToComponentId{};
    std::vector<ComponentType> components{};

    template<typename... Args>
    requires(std::constructible_from<ComponentType, Args...>)
    auto AddComponent(EntityId entityId, Args&&... args) -> ComponentId {
        if (entityIdToComponentId.contains(entityId)) ThrowMessage("ERROR", "Attempted to add component to entity that already has it");
        auto componentId = components.size();
        components.emplace_back(std::forward<Args...>(args...));
        entityIdToComponentId.emplace(entityId, componentId);
        return componentId;
    }

    auto GetComponent(this auto&& self, EntityId entityId) -> ComponentId {
        auto iter = std::forward<decltype(self)>(self.entityIdToComponentId).find(entityId);
        if (iter == self.entityIdToComponentId.end()) ThrowMessage("ERROR", "Attempted to access component of entity that doesn't have it");
        return iter->value;
    }
};

template <typename... ComponentTypes>
struct ECSManager {
    [[nodiscard]] auto FreshEntity() noexcept -> EntityId { 
        return nextId++;
    }

private:
    EntityId nextId{};
    std::tuple<ComponentManager<ComponentTypes>...> componentManagers{}; 
};
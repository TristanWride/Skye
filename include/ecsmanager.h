#pragma once

#include "debugutils.h"
#include "opaquetypes.h"
#include "componentmanagers.h"

#include <concepts>
#include <cstdint>
#include <tuple>
#include <vector>
#include <deque>

template <typename... ComponentTypes>
requires(ComponentManager<typename FindComponentManager<ComponentTypes>::ManagerType> && ...)
struct ECSManager {
    [[nodiscard]] auto FreshEntity() noexcept -> EntityId { 
        if (freedIds.empty())
            return nextId++;
        const auto id = freedIds.front();
        freedIds.pop_front();
        return id;
    }

    template <typename ComponentType>
    decltype(auto) GetComponentManager(this auto&& self) {
        return std::get<FindComponentManager<ComponentType>::ManagerType>(std::forward<decltype(self)>(self).componentManagers);
    }

private:
    EntityId nextId{};
    std::deque<EntityId> freedIds{};
    std::tuple<typename FindComponentManager<ComponentTypes>::ManagerType...> componentManagers{}; 
};
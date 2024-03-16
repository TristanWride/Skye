#pragma once

#include "componentmanagers.h"
#include "debugutils.h"

#include <bitset>
#include <concepts>
#include <cstdint>
#include <optional>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <unordered_map>

static constexpr auto MAX_NUM_ENTITIES = EntityId{10'000};

template <typename... CMs>
requires (ComponentManager<CMs> && ...)
class ECSManager {
public:
    static constexpr auto NumComponents = sizeof...(CMs);
    using ComponentTypes = std::tuple<typename CMs::ComponentType...>;
    using ComponentBitset = std::bitset<NumComponents>;

private:
    std::tuple<CMs...> componentManagers{};
    std::vector<std::optional<ComponentBitset>> entityBits;

public:
    [[nodiscard]] auto NewEntity() -> std::optional<EntityId> {
        auto iter = std::ranges::find_if_not(entityBits, [](const auto& bits) { return bits.has_value(); });
        if (iter != entityBits.end()) return static_cast<EntityId>(iter - entityBits.begin());
        if (entityBits.size() >= MAX_NUM_ENTITIES) return std::nullopt;
        entityBits.emplace_back(ComponentBitset{});
        return static_cast<EntityId>(entityBits.size() - 1);
    }

    template <typename... Comps>
    requires SupportsComponents<ECSManager<CMs...>, Comps...>
    static consteval auto MakeComponentBitset() noexcept -> ComponentBitset {
        auto result = ComponentBitset{};
        (result.set(ComponentIndex<Comps>()), ...);
        return result;
    }

    template <typename Comp, std::size_t I = 0u, bool Found = false, bool Failed = false>
    static consteval auto HasComponent() noexcept -> bool {
        if constexpr (I >= std::tuple_size_v<ComponentTypes>) {
            return Found && !Failed;
        } else if constexpr (std::is_same_v<std::tuple_element_t<I, ComponentTypes>, Comp>) {
            return HasComponent<Comp, I + 1, true, Found>();
        } else {
            return HasComponent<Comp, I + 1, Found, Failed>();
        }
    }

    template <typename Comp, std::size_t I = 0u, bool Found = false, std::size_t FoundI = 0u>
    requires SupportsComponent<ECSManager<CMs...>, Comp>
    static consteval auto ComponentIndex() noexcept -> std::size_t {
        if constexpr (I >= std::tuple_size_v<ComponentTypes>) {
            static_assert (Found);
            return FoundI;
        } else if constexpr (std::is_same_v<std::tuple_element_t<I, ComponentTypes>, Comp>) {
            static_assert (!Found);
            return ComponentIndex<Comp, I + 1, true, I>();
        } else {
            return ComponentIndex<Comp, I + 1, Found, FoundI>();
        }
    }

    auto DeleteEntity(EntityId id) -> void {
        entityBits[id] = std::nullopt;
    }

    template <typename Comp>
    requires SupportsComponent<ECSManager<CMs...>, Comp>
    decltype(auto) GetComponent(this auto&& self, EntityId id) {
        return std::get<ComponentIndex<Comp>()>(std::forward<decltype(self)>(self).componentManagers).Get(id);
    }

    template <typename Comp, typename... Args>
    requires SupportsComponent<ECSManager<CMs...>, Comp> && std::constructible_from<Comp, Args...>
    auto NewComponent(EntityId id, Args&&... args) -> void {
        entityBits[id]->set(ComponentIndex<Comp>());
        std::get<ComponentIndex<Comp>()>(componentManagers).New(id, std::forward<Args>(args)...);
    }

    template <typename... Comps>
    requires SupportsComponents<ECSManager<CMs...>, Comps...>
    decltype(auto) GetAll(this auto&& self) {
        static constexpr auto compBitset = MakeComponentBitset<Comps...>();
        return std::forward<decltype(self)>(self).entityBits 
            | std::views::enumerate
            | std::views::filter([] (const auto& entity) {
                const auto& entityBits = std::get<1>(entity);
                if (!entityBits.has_value()) return false;
                return (compBitset & entityBits.value()) == compBitset;
            })
            | std::views::transform([&self] (const auto& entity) {
                auto id = static_cast<EntityId>(std::get<0>(entity));
                return std::make_tuple(id, std::ref(std::forward<decltype(self)>(self).GetComponent<Comps>(id))...);
            });
    }
};

template <typename T>
struct BasicCompManager {
    using ComponentType = T;
    std::unordered_map<EntityId, T> map;

    template <typename... Args>
    auto New(EntityId id, Args&&... args) {
        map.try_emplace(id, std::forward<Args>(args)...);
    }

    decltype(auto) Get(this auto&& self, EntityId id) {
        return std::forward<decltype(self)>(self).map.at(id);
    }
};

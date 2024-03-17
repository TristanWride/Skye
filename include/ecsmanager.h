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
class ECSManager {
public:
    static constexpr auto NComponents = sizeof...(CMs);
    using ComponentTypes = std::tuple<typename CMs::ComponentType...>;
    using ComponentBitset = std::bitset<NComponents>;

    [[nodiscard]] consteval auto NumComponentSlots() const noexcept {
        return NComponents;
    }

    template <typename... Comps>
    [[nodiscard]] static consteval auto MakeComponentBitset() noexcept -> ComponentBitset {
        auto result = ComponentBitset{};
        (result.set(ComponentIndex<Comps>()), ...);
        return result;
    }

    template <typename Comp, std::size_t I = 0u, bool Found = false, bool Failed = false>
    [[nodiscard]] static consteval auto HasComponentManager() noexcept -> bool {
        if constexpr (I >= std::tuple_size_v<ComponentTypes>) {
            return Found && !Failed;
        } else if constexpr (std::is_same_v<std::tuple_element_t<I, ComponentTypes>, Comp>) {
            return HasComponentManager<Comp, I + 1, true, Found>();
        } else {
            return HasComponentManager<Comp, I + 1, Found, Failed>();
        }
    }

    template <typename Comp, std::size_t I = 0u, bool Found = false, std::size_t FoundI = 0u>
    [[nodiscard]] static consteval auto ComponentIndex() noexcept {
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
    [[nodiscard]] auto HasComponents(EntityId id) const -> bool {
        static constexpr auto requiredBits = MakeComponentBitset<Comps...>();
        if (!IsValidEntity(id)) return false;
        const auto& actualBits = entityBits.at(id).value();
        return (MakeComponentBitset<Comps...>() & actualBits) == MakeComponentBitset<Comps...>();
    }

    [[nodiscard]] auto IsValidEntity(EntityId id) const -> bool {
        if (id >= NumEntitySlots()) return false;
        return entityBits.at(id).has_value();
    }

    auto DeleteEntity(EntityId id) -> void {
        std::apply([&](auto&&... cms) {
            const auto& eBits = entityBits[id].value();
            auto compIndex = 0;

            ([&]() {
                if (eBits[compIndex]) cms.Delete(id);
                ++compIndex;
            }(), ...);
        }, componentManagers);
        entityBits[id] = std::nullopt;
    }

    template <typename Comp>
    [[nodiscard]] auto&& GetComponent(this auto&& self, EntityId id) {
        return std::get<ComponentIndex<Comp>()>(std::forward<decltype(self)>(self).componentManagers).Get(id);
    }

    template <typename Comp>
    [[nodiscard]] auto&& GetComponentManager(this auto&& self) {
        return std::get<ComponentIndex<Comp>()>(std::forward<decltype(self)>(self).componentManagers);
    }

    template <typename Comp, typename... Args>
    auto NewComponent(EntityId id, Args&&... args) -> void {
        entityBits[id]->set(ComponentIndex<Comp>());
        std::get<ComponentIndex<Comp>()>(componentManagers).New(id, std::forward<Args>(args)...);
    }

    template <typename... Comps>
    [[nodiscard]] auto GetAll(this auto&& self) {
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

    [[nodiscard]] constexpr auto NumEntitySlots() const noexcept {
        return entityBits.size();
    }
};

template <typename T>
struct BasicCompManager {
    using ComponentType = T;
    std::unordered_map<EntityId, T> map;

    template <typename... Args>
    auto New(EntityId id, Args&&... args) -> void {
        map.try_emplace(id, std::forward<Args>(args)...);
    }

    [[nodiscard]] auto&& Get(this auto&& self, EntityId id) {
        return std::forward<decltype(self)>(self).map.at(id);
    }

    [[nodiscard]] auto HasEntity(EntityId id) const -> bool {
        auto iter = map.find(id);
        return iter != map.cend();
    }

    auto Delete(EntityId id) -> bool {
        auto iter = map.find(id);
        if (iter == map.end()) return false;
        map.erase(iter);
        return true;
    }
};

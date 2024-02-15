#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "debugutils.h"
#include "ecsmanager.h"
#include "window.h"

#include <format>
#include <string>
#include <unordered_map>

auto ErrorCallback(int error, const char* description) -> void {
    DebugMessage("ERROR", std::format("error code {}: {}", error, description));
}

auto GlobalSetup() -> void {
    if (glfwInit() != GLFW_TRUE) ThrowMessage("ERROR", "Failed to initialize GLFW");
    glfwSetErrorCallback(ErrorCallback);
}

auto GlobalCleanup() -> void {
    glfwTerminate();
}

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

auto main() -> int {
    struct Environment {
        Environment() { GlobalSetup(); }
        ~Environment() { GlobalCleanup(); }
    }; 

    auto env = Environment{};
    auto window = Window();

    if (!gladLoadGL()) ThrowMessage("ERROR", "Failed to initialise GLAD");

    auto ecs = ECSManager<
        BasicCompManager<int>, 
        BasicCompManager<double>, 
        BasicCompManager<std::string>,
        BasicCompManager<bool>
    >{};

    for (auto i = 0; i < 20; ++i) {
        auto ent = ecs.NewEntity().value();
        if (i % 2 == 0) ecs.NewComponent<int>(ent, i);
        if (i % 3 == 0) ecs.NewComponent<std::string>(ent, i, 'x');
    }

    for (auto [id, x] : std::as_const(ecs).GetAll<int>()) {
        DebugMessage("INFO", std::format("Entity {:>4d}: IntComp {:>4d}", id, x));
    }

    for (auto [id, x] : std::as_const(ecs).GetAll<std::string>()) {
        DebugMessage("INFO", std::format("Entity {:>4d}: StrComp {:<21}", id, x));
    }

    for (auto [id, s, x] : std::as_const(ecs).GetAll<std::string, int>()) {
        DebugMessage("INFO", std::format("Entity {:>4d}: StrComp {:<21}   IntComp {:>4d}", id, s, x));
    }

    while (!glfwWindowShouldClose(window.window)) {
        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}
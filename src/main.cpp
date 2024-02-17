#include "debugutils.h"
#include "ecsmanager.h"
#include "window.h"
#include "meshcomponent.h"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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

    auto mesh = Mesh::ReadObj(DATA_DIR "tris.obj");

    const auto& rend = mesh.GetRenderable();

    while (!glfwWindowShouldClose(window.window)) {
        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}
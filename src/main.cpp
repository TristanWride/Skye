#include "debugutils.h"
#include "window.h"
#include "ecsmanager.h"
#include "meshcomponent.h"
#include "cameracomponent.h"
#include "transformcomponent.h"
#include "renderer.h"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <format>
#include <string>
#include <unordered_map>

#include <iostream>

auto ErrorCallback(int error, const char* description) noexcept -> void {
    DebugMessage("ERROR", std::format("error code {}: {}", error, description));
}

auto GlobalSetup() -> void {
    if (glfwInit() != GLFW_TRUE) ThrowMessage("ERROR", "Failed to initialize GLFW");
    glfwSetErrorCallback(ErrorCallback);
}

auto GlobalCleanup() noexcept -> void {
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
        Environment() noexcept { GlobalSetup(); }
        ~Environment() noexcept { GlobalCleanup(); }
    }; 

    auto env = Environment{};
    auto window = Window();

    if (!gladLoadGL()) ThrowMessage("ERROR", "Failed to initialise GLAD");

    auto ecs = ECSManager<
        BasicCompManager<MeshComponent>,
        BasicCompManager<CameraComponent>,
        BasicCompManager<TransformComponent>
    >{};

    auto renderer = Renderer{ecs, window};
    auto renderMesh = ecs.NewEntity().value();
    ecs.NewComponent<MeshComponent>(renderMesh, Mesh::ReadObj(DATA_DIR "tris.obj"));

    auto camera = ecs.NewEntity().value();
    ecs.NewComponent<CameraComponent>(camera, 45.0f, window.GetAspectRatio(), 0.1f, 100.0f);
    auto proj = ecs.GetComponent<CameraComponent>(camera).GetProjection();

    ecs.NewComponent<TransformComponent>(camera, glm::vec3(1.0f), glm::mat4(1.0f), glm::vec3(0.0, 0.0, 10.0));
    renderer.activeCamera = camera;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    while (!glfwWindowShouldClose(window.window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer.RenderMeshes();

        glfwSwapBuffers(window.window);
        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}
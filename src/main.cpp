#include "debugutils.h"
#include "window.h"
#include "ecsmanager.h"
#include "meshcomponent.h"
#include "cameracomponent.h"
#include "transformcomponent.h"
#include "renderer.h"
#include "inputcomponent.h"

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

[[nodiscard]] auto GlobalSetup() -> Window {
    if (glfwInit() != GLFW_TRUE) ThrowMessage("ERROR", "Failed to initialize GLFW");
    glfwSetErrorCallback(ErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto window = Window();

    if (!gladLoadGL()) ThrowMessage("ERROR", "Failed to initialise GLAD");

    return window;
}

auto GlobalCleanup() noexcept -> void {
    glfwTerminate();
}

auto main() -> int {
    struct Environment {
        Window window;

        Environment() noexcept 
            : window{ GlobalSetup() }
        {}

        ~Environment() noexcept { 
            window.Destroy();
            GlobalCleanup();
        }
    }; 

    auto env = Environment{};

    auto ecs = ECSManager<
        BasicCompManager<MeshComponent>,
        BasicCompManager<CameraComponent>,
        BasicCompManager<TransformComponent>
    >{};

    auto renderer = Renderer{ecs, env.window};
    auto renderMesh = ecs.NewEntity().value();
    ecs.NewComponent<MeshComponent>(renderMesh, Mesh::ReadObj(DATA_DIR "tris.obj"));

    auto camera = ecs.NewEntity().value();
    ecs.NewComponent<CameraComponent>(camera, 45.0f, env.window.GetAspectRatio(), 0.1f, 100.0f);
    auto proj = ecs.GetComponent<CameraComponent>(camera).GetProjection();

    ecs.NewComponent<TransformComponent>(camera, glm::vec3(1.0f), glm::mat4(1.0f), glm::vec3(0.0, 0.0, 10.0));
    renderer.activeCamera = camera;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    while (!glfwWindowShouldClose(env.window.window.get())) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer.RenderMeshes();

        glfwSwapBuffers(env.window.window.get());
        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}
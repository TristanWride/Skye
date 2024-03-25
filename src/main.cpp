#include "cameracomponent.h"
#include "ecsmanager.h"
#include "inputcomponent.h"
#include "meshcomponent.h"
#include "renderer.h"
#include "transformcomponent.h"
#include "window.h"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

auto main() noexcept -> int {
    Window::Initialize();

    auto ecs = ECSManager<
        BasicCompManager<MeshComponent>,
        BasicCompManager<CameraComponent>,
        BasicCompManager<TransformComponent>
    >{};

    GLFWInputAdapter::Initialize(Window::GetWindow());
    auto inputSystem = InputSystem{GLFWInputAdapter::GetInstance()};
    auto dbgIC = DebugInputComponent{};
    inputSystem.RegisterInputComponent(dbgIC);

    auto renderer = Renderer{ecs};
    auto renderMesh = ecs.NewEntity().value(); // NOLINT
    ecs.NewComponent<MeshComponent>(renderMesh, Mesh::ReadObj(DATA_DIR "tris.obj"));

    auto camera = ecs.NewEntity().value(); // NOLINT
    ecs.NewComponent<CameraComponent>(camera, 45.0F, Window::GetAspectRatio(), 0.1F, 100.0F); // NOLINT (cppcoreguidelines-avoid-magic-numbers)

    ecs.NewComponent<TransformComponent>(camera, glm::vec3(1.0F), glm::mat4(1.0F), glm::vec3(0.0F, 0.0F, 10.0F)); // NOLINT (cppcoreguidelines-avoid-magic-numbers)

    renderer.activeCamera = camera;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    while (glfwWindowShouldClose(Window::GetWindow()) != GLFW_TRUE) {
        glClearColor(0.2F, 0.3F, 0.3F, 1.0F); // NOLINT (cppcoreguidelines-avoid-magic-numbers)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer.RenderMeshes();

        glfwSwapBuffers(Window::GetWindow());
        glfwPollEvents();
    }

    return 0;
}
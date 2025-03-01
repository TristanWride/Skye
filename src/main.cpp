#include "ecsmanager.h"
#include "inputcomponent.h"
#include "window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdexcept>

auto main() noexcept -> int try {
    Window::Initialize();

    auto ecs = ECSManager<>{};

    GLFWInputAdapter::Initialize(Window::GetWindow());
    auto inputSystem = InputSystem{GLFWInputAdapter::GetInstance()};
    auto dbgIC = DebugInputComponent{};
    inputSystem.RegisterInputComponent(dbgIC);

    while (glfwWindowShouldClose(Window::GetWindow()) != GLFW_TRUE) {
        glfwSwapBuffers(Window::GetWindow());
        glfwPollEvents();
    }

    return 0;
} catch (const std::runtime_error& e) {
    DebugMessage("CRASH", e.what());
}

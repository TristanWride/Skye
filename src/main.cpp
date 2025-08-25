#include "ecsmanager.hpp"
#include "inputcomponent.hpp"
#include "window.hpp"

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

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

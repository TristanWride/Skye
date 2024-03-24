#pragma once

#include "debugutils.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <memory>

class Window {
private:
    Window();

    std::unique_ptr<GLFWwindow, decltype([](GLFWwindow* window) {
        DebugMessage("INFO", "Destroying window");
        glfwDestroyWindow(window);
    })> window;

    [[nodiscard]] static auto GetInstance() noexcept -> Window&;

public:
    static void Initialize();
    static void Terminate() noexcept;
    [[nodiscard]] static auto GetWindow() noexcept -> GLFWwindow*;
    [[nodiscard]] static auto GetAspectRatio() noexcept -> float;
};
#pragma once

#include "debugutils.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <memory>

class Window {
public:
    Window();

    auto GetAspectRatio() const noexcept -> float;
    auto Destroy() noexcept -> void;

    std::unique_ptr<GLFWwindow, decltype([](GLFWwindow* window) {
        DebugMessage("INFO", "Destroying window");
        glfwDestroyWindow(window);
    })> window;
};
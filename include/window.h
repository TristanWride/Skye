#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class Window {
public:
    Window();
    ~Window() noexcept;

    Window(const Window& other) = delete;
    Window(Window&& other) = delete;
    Window& operator=(const Window& other) = delete;
    Window& operator=(Window&& other) = delete;

    auto GetAspectRatio() const noexcept -> float;

    GLFWwindow* window;
};
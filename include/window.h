#pragma once

#include <GLFW/glfw3.h>

class Window {
public:
    Window();
    ~Window();

    Window(const Window& other) = delete;
    Window(Window&& other) = delete;
    Window& operator=(const Window& other) = delete;
    Window& operator=(Window&& other) = delete;

    GLFWwindow* window;
};
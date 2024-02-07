#include "window.h"

#include "debugutils.h"

Window::Window() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    window = glfwCreateWindow(1600, 900, "Project", NULL, NULL);
    if (!window) {
        DebugMessage("ERROR", "Failed to create window");
        return;
    }

    glfwMakeContextCurrent(window);
}

Window::~Window() {
    glfwDestroyWindow(window);
}
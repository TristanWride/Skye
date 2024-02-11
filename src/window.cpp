#include "window.h"

#include "debugutils.h"

#include <glad/glad.h>

auto FramBufferSizeCallback(GLFWwindow* window, int width, int height) -> void {
    glViewport(0, 0, width, height);
}

Window::Window() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1600, 900, "Project", NULL, NULL);
    if (!window) {
        ThrowMessage("ERROR", "Failed to create window");
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, &FramBufferSizeCallback);
}

Window::~Window() {
    glfwDestroyWindow(window);
}
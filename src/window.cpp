#include "window.h"

#include "debugutils.h"

#include <glad/glad.h>

auto FrameBufferSizeCallback([[maybe_unused]] GLFWwindow* window, int width, int height) noexcept -> void {
    glViewport(0, 0, width, height);
}

Window::Window() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1600, 900, "Project", NULL, NULL);

    if (!window) ThrowMessage("ERROR", "Failed to create window");

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, &FrameBufferSizeCallback);

    glfwSwapInterval(1);
}

Window::~Window() noexcept {
    glfwDestroyWindow(window);
}

auto Window::GetAspectRatio() const noexcept -> float {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    return static_cast<float>(width) / height;
}
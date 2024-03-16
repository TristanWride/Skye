#include "window.h"

#include <glad/glad.h>

#include "debugutils.h"

auto FrameBufferSizeCallback([[maybe_unused]] GLFWwindow* window, int width, int height) noexcept -> void {
    glViewport(0, 0, width, height);
}

Window::Window()
    : window{glfwCreateWindow(1600, 900, "Project", NULL, NULL)}
{
    if (!window) ThrowMessage("ERROR", "Failed to create window");

    glfwMakeContextCurrent(window.get());
    glfwSetFramebufferSizeCallback(window.get(), &FrameBufferSizeCallback);

    glfwSwapInterval(1);
}

auto Window::GetAspectRatio() const noexcept -> float {
    int width, height;
    glfwGetWindowSize(window.get(), &width, &height);
    return static_cast<float>(width) / height;
}

auto Window::Destroy() noexcept -> void {
    window.release();
}

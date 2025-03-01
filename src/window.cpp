#include "debugutils.hpp"
#include "window.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <format>

constexpr unsigned int WINDOW_WIDTH = 1600U;
constexpr unsigned int WINDOW_HEIGHT = 900U;

void ErrorCallback(int error, const char* description) noexcept {
    DebugMessage("ERROR", "error code {}: {}", error, description);
}

auto GlobalCleanup() noexcept -> void {
    glfwTerminate();
}

Window::Window()
    : window{glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Project", nullptr, nullptr)}
{
    DebugMessage("INFO", "Creating window");
}

auto Window::GetInstance() noexcept -> Window& {
    static Window window{};
    return window;
}

void Window::Initialize() {
    DebugMessage("INFO", "Initializing GLFW");

    if (glfwInit() != GLFW_TRUE) { ThrowMessage("ERROR", "Failed to initialize GLFW"); }
    glfwSetErrorCallback(ErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto& window = GetInstance();

    if (!window.window) { ThrowMessage("ERROR", "Failed to create window"); }

    glfwMakeContextCurrent(GetWindow());

    glfwSwapInterval(1);
}

void Window::Terminate() noexcept {
    GetInstance().window.reset();
}

auto Window::GetWindow() noexcept -> GLFWwindow* {
    return GetInstance().window.get();
}

auto Window::GetAspectRatio() noexcept -> float {
    int width{}; int height{};
    glfwGetWindowSize(GetWindow(), &width, &height);
    return static_cast<float>(width) / static_cast<float>(height);
}

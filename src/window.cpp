#include "window.h"

#include "debugutils.h"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <format>

constexpr unsigned int OPENGL_MAJOR_VERSION = 4U;
constexpr unsigned int OPENGL_MINOR_VERSION = 6U;

constexpr unsigned int WINDOW_WIDTH = 1600U;
constexpr unsigned int WINDOW_HEIGHT = 900U;

void ErrorCallback(int error, const char* description) noexcept {
    DebugMessage("ERROR", std::format("error code {}: {}", error, description));
}

auto GlobalCleanup() noexcept -> void {
    glfwTerminate();
}

auto FrameBufferSizeCallback([[maybe_unused]] GLFWwindow* window, int width, int height) noexcept -> void {
    glViewport(0, 0, width, height);
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_MAJOR_VERSION);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_MINOR_VERSION);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto& window = GetInstance();

    if (!window.window) { ThrowMessage("ERROR", "Failed to create window"); }

    glfwMakeContextCurrent(GetWindow());

    if (!static_cast<bool>(gladLoadGL())) { ThrowMessage("ERROR", "Failed to initialise GLAD"); }

    glfwSetFramebufferSizeCallback(GetWindow(), &FrameBufferSizeCallback);
    glfwSwapInterval(1);
}

void Window::Terminate() noexcept {
    (void) GetInstance().window.release();
}

auto Window::GetWindow() noexcept -> GLFWwindow* {
    return GetInstance().window.get();
}

auto Window::GetAspectRatio() noexcept -> float {
    int width{}; int height{};
    glfwGetWindowSize(GetWindow(), &width, &height);
    return static_cast<float>(width) / static_cast<float>(height);
}

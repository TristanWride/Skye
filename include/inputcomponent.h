#pragma once
#include "debugutils.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <concepts>

struct InputComponent {
    virtual void KeyCallback(int key, int scancode, int action, int mods) noexcept = 0;
    virtual void MousePosCallback(double xPos, double yPos) noexcept = 0;
    virtual void MouseBtnCallback(int button, int action, int mods) = 0;
};

struct GLFWInputAdapter {
    using KeyCallbackFn = GLFWkeyfun;    
    using MousePosCallbackFn = GLFWcursorposfun;
    using MouseBtnCallbackFn = GLFWmousebuttonfun;

private:
    InputComponent* currentComponent;
    GLFWwindow* window;

    [[nodiscard]] GLFWInputAdapter() noexcept = default;

    GLFWInputAdapter(const GLFWInputAdapter&) = delete;
    auto operator=(const GLFWInputAdapter&) -> GLFWInputAdapter& = delete;
    GLFWInputAdapter(GLFWInputAdapter&&) = delete;
    auto operator=(GLFWInputAdapter&&) -> GLFWInputAdapter& = delete;

public:
    static auto Initialize(GLFWwindow* window) noexcept -> GLFWInputAdapter& {
        auto& adapter = GetInstance();
        adapter.window = window;
        return adapter;
    }

    [[nodiscard]] static auto GetInstance() noexcept -> GLFWInputAdapter& {
        static GLFWInputAdapter adapter{};
        return adapter;
    }

    inline void RegisterInputComponent(InputComponent& component) {
        if (window == nullptr) { ThrowMessage("ERROR", "Trying to register input with no window set"); }

        currentComponent = &component;

        glfwSetKeyCallback(window, static_cast<KeyCallbackFn>([](auto* window, int key, int scancode, int action, int mods) { GetInstance().currentComponent->KeyCallback(key, scancode, action, mods); }));
        glfwSetCursorPosCallback(window, static_cast<MousePosCallbackFn>([](auto* window, double xPos, double yPos) { GetInstance().currentComponent->MousePosCallback(xPos, yPos); }));
        glfwSetMouseButtonCallback(window, static_cast<MouseBtnCallbackFn>([](auto* window, int button, int action, int mods) { GetInstance().currentComponent->MouseBtnCallback(button, action, mods); }));
    }

    inline void Update() noexcept { glfwPollEvents(); }
};

template <typename InputAdapter>
struct InputSystem {
    InputAdapter& adapter;

    inline void RegisterInputComponent(InputComponent& component) { adapter.RegisterInputComponent(component); }
    inline void Update() noexcept { adapter.Update(); }
};

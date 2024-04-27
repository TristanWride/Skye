#pragma once
#include "debugutils.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <concepts>

struct InputComponent {
    virtual inline void OnAttach() noexcept {}
    virtual inline void KeyCallback([[maybe_unused]] int key, [[maybe_unused]] int scancode, [[maybe_unused]] int action, [[maybe_unused]] int mods) noexcept {}
    virtual inline void CharCallback([[maybe_unused]] unsigned int c) noexcept {}
    virtual inline void MousePosCallback([[maybe_unused]] double xPos, [[maybe_unused]] double yPos) noexcept {}
    virtual inline void MouseBtnCallback([[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods) noexcept {}
    virtual inline void MouseScrollCallback([[maybe_unused]] double xScroll, [[maybe_unused]] double yScroll) noexcept {}
};

struct GLFWInputAdapter {
    using KeyCallbackFn = GLFWkeyfun;    
    using CharCallbackFn = GLFWcharfun;
    using MousePosCallbackFn = GLFWcursorposfun;
    using MouseBtnCallbackFn = GLFWmousebuttonfun;
    using MouseScrollCallbackFn = GLFWscrollfun;

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
        if (window == nullptr) ThrowMessage("ERROR", "Trying to register input with no window set");

        currentComponent = &component;

        component.OnAttach();

        glfwSetKeyCallback(window, static_cast<KeyCallbackFn>([](auto*, int key, int scancode, int action, int mods) { GetInstance().currentComponent->KeyCallback(key, scancode, action, mods); }));
        glfwSetCharCallback(window, static_cast<CharCallbackFn>([](auto*, unsigned int codePoint) { GetInstance().currentComponent->CharCallback(codePoint); }));
        glfwSetCursorPosCallback(window, static_cast<MousePosCallbackFn>([](auto*, double xPos, double yPos) { GetInstance().currentComponent->MousePosCallback(xPos, yPos); }));
        glfwSetMouseButtonCallback(window, static_cast<MouseBtnCallbackFn>([](auto*, int button, int action, int mods) { GetInstance().currentComponent->MouseBtnCallback(button, action, mods); }));
        glfwSetScrollCallback(window, static_cast<MouseScrollCallbackFn>([](auto*, double xScroll, double yScroll) { GetInstance().currentComponent->MouseScrollCallback(xScroll, yScroll); }));
    }

    inline void Update() noexcept { glfwPollEvents(); }
};

template <typename InputAdapter>
struct InputSystem {
    InputAdapter& adapter;

    inline void RegisterInputComponent(InputComponent& component) { adapter.RegisterInputComponent(component); }
    inline void Update() noexcept { adapter.Update(); }
};

struct DebugInputComponent : public InputComponent {
    inline void OnAttach() noexcept override {
        DebugMessage("INFO", "Debug input component attached");
    }

    inline void KeyCallback(int key, int scancode, int action, [[maybe_unused]] int mods) noexcept override {
        const auto name = glfwGetKeyName(key, scancode);
        const auto actionName = [&] {
            if (action == GLFW_PRESS) return "PRESS";
            else if (action == GLFW_REPEAT) return "REPEAT";
            else if (action == GLFW_RELEASE) return "RELEASE";
            
            return "UNKNOWN";
        }();
        if (name != nullptr) { DebugMessage("INFO", "[{:<10}] : [{:<10}]   [{:<10}]", "KEY_INPUT", name, actionName); }
    }

    inline void MousePosCallback(double xPos, double yPos) noexcept override {
        DebugMessage("INFO", "[{:<10}] : [{:<10.5}]   [{:<10.5}]", "CURSOR", xPos, yPos); 
    }

    inline void MouseBtnCallback(int button, int action, [[maybe_unused]] int mods) noexcept override {
        const auto actionName = [&] {
            if (action == GLFW_PRESS) return "PRESS";
            else if (action == GLFW_REPEAT) return "REPEAT";
            else if (action == GLFW_RELEASE) return "RELEASE";
            
            return "UNKNOWN";
        }();
        DebugMessage("INFO", "[{:10}] : [{:<10}]   [{:<10}]", "MOUSE_BTN", button, actionName);
    }

    inline void MouseScrollCallback(double xScroll, double yScroll) noexcept override {
        DebugMessage("INFO", "[{:<10}] : [{:<10.5}]   [{:<10.5}]", "SCROLL", xScroll, yScroll); 
    }
};

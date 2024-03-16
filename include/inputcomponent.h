#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>
#include <concepts>

struct InputHandler {
    virtual void InputListener() noexcept = 0;
    virtual void Update() noexcept = 0;
    virtual ~InputHandler() noexcept = 0;
};

struct InputComponent : public InputHandler {
    std::unique_ptr<InputHandler> inner;

    template <typename InputHandlerT, typename... Args>
    requires std::derived_from<InputHandlerT, InputHandler> && std::constructible_from<InputHandlerT, Args...>
    InputComponent(Args&&... args)
        : inner{std::make_unique(std::forward<Args>(args)...)} 
    {}

    inline virtual void InputListener() noexcept {
        inner->InputListener();
    }

    inline virtual void Update() noexcept {
        inner->Update();
    }

    virtual ~InputComponent() noexcept = default;
};

template <typename GlobalInputState>
struct InputSystem {
    GlobalInputState& state;


};


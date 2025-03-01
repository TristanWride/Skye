#pragma once

#include <print>
#include <utility>
#include <stdexcept>

#ifndef NDEBUG
inline static constexpr bool DebugRunning = true;
#else
inline static constexpr bool DebugRunning = false;
#endif

template <class S, class T>
auto DebugMessage([[maybe_unused]] S&& debugLevel, [[maybe_unused]] T&& message) noexcept -> void {
    if constexpr (DebugRunning) {
        std::println(
            "[{:<7}]: {}", 
            std::forward<S>(debugLevel), 
            std::forward<T>(message)
        );
    } 
}

template <class S, class... Args>
auto DebugMessage(S&& debugLevel, std::format_string<Args...> format, Args&&... args) noexcept -> void {
    DebugMessage(std::forward<S>(debugLevel), std::format(format, std::forward<Args>(args)...));
}

template <class S, class T>
[[noreturn]] auto ThrowMessage([[maybe_unused]] S&& debugLevel, T&& message) -> void {
    if constexpr (DebugRunning) std::println("[{:<7}]: {}", std::forward<S>(debugLevel), message);
    
    throw std::runtime_error(message);
}

template <class S, class... Args>
[[noreturn]] auto ThrowMessage(S&& debugLevel, std::format_string<Args...> format, Args&&... args) noexcept -> void {
    ThrowMessage(std::forward<S>(debugLevel), std::format(format, std::forward<Args>(args)...));
}

template <class S, class... Args>
[[noreturn]] auto DebugOnlyThrowMessage([[maybe_unused]] S&& debugLevel, [[maybe_unused]] std::format_string<Args...> format, [[maybe_unused]] Args&&... args) -> void {
    if constexpr (DebugRunning) ThrowMessage(std::forward<S>(debugLevel), format, std::forward<Args>(args)...);

    std::unreachable();
}

template <class S, class T>
[[noreturn]] auto DebugOnlyThrowMessage([[maybe_unused]] S&& debugLevel, [[maybe_unused]] T&& message) -> void {
    if constexpr (DebugRunning) ThrowMessage(std::forward<S>(debugLevel), std::forward<T>(message));

    std::unreachable();
}

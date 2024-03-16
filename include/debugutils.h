#pragma once

#ifndef NDEBUG
#include <print>
#include <utility>
#endif

#ifndef NDEBUG
inline constexpr bool DebugRunning = true;
#else
inline constexpr bool DebugRunning = false;
#endif

template <class S, class T>
auto DebugMessage(S&& debugLevel, T&& message) noexcept -> void {
    if constexpr (DebugRunning) {
        std::println(
            "[{:<7}]: {}", 
            std::forward<S>(debugLevel), 
            std::forward<T>(message)
        );
    } 
}

template <class S, class T>
auto ThrowMessage(S&& debugLevel, T&& message) -> void {
    if constexpr (DebugRunning) {
        std::println(
            "[{:<7}]: {}", 
            std::forward<S>(debugLevel), 
            std::forward<T>(message)
        );
    }
    
    throw std::runtime_error(std::format(
        "[{:<7}]: {}", 
        std::forward<S>(debugLevel), 
        std::forward<T>(message)
    ));
}
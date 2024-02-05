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
auto DebugMessage(S&& debugLevel, T&& message) -> void {
    if constexpr (DebugRunning) {
        std::print(
            "[{:<7}]: {}\n", 
            std::forward<S>(debugLevel), 
            std::forward<T>(message)
        );
    } else {
        // Do nothing
    }
}
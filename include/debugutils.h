#pragma once

#ifndef NDEBUG
#include <print>
#include <utility>
#endif

#ifndef NDEBUG
inline static constexpr bool DebugRunning = true;
#else
inline static constexpr bool DebugRunning = false;
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
[[noreturn]] auto ThrowMessage(S&& debugLevel, T&& message) -> void {
    const auto messageStr = std::format(
            "[{:<7}]: {}", 
            std::forward<S>(debugLevel), 
            std::forward<T>(message)
    );

    if constexpr (DebugRunning) std::println("{}", messageStr);
    
    throw std::runtime_error(messageStr);
}

template <class S, class F, class... Args>
auto DebugFmtMessage(S&& debugLevel, F&& format, Args&&... args) noexcept -> void {
    DebugMessage(std::forward<S>(debugLevel), std::format(std::forward<F>(format), std::forward<Args>(args)...));
}

template <class S, class F, class... Args>
auto ThrowFmtMessage(S&& debugLevel, F&& format, Args&&... args) noexcept -> void {
    ThrowMessage(std::forward<S>(debugLevel), std::format(std::forward<F>(format), std::forward<Args>(args)...));
}

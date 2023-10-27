#pragma once
#include <iostream>
#include <utility>

namespace Logger {
template <typename... Args>
static inline void Error(Args&&... args) noexcept {
    std::cout << std::boolalpha << "\x1B[31m";
    ((std::cout << std::forward<Args>(args) << ' '), ...);
    std::cout << "\033[0m" << std::endl;
}

template <typename... Args>
static inline void Succsess(Args&&... args) noexcept {
    std::cout << std::boolalpha << "\x1B[32m";
    ((std::cout << std::forward<Args>(args) << ' '), ...);
    std::cout << "\033[0m" << std::endl;
}

template <typename... Args>
static inline void Warning(Args&&... args) noexcept {
    std::cout << std::boolalpha << "\x1B[33m";
    ((std::cout << std::forward<Args>(args) << ' '), ...);
    std::cout << "\033[0m" << std::endl;
}

template <typename... Args>
static inline void Print(Args&&... args) noexcept {
    std::cout << std::boolalpha;
    ((std::cout << std::forward<Args>(args) << ' '), ...) << std::endl;
}
}  // namespace Logger

#ifdef _DEBUG
#define D_PRINT(...) \
    Logger::Print(__VA_ARGS__)

#define D_WARN(...) \
    Logger::Warning(__VA_ARGS__)

#define D_ERR(...) \
    Logger::Error(__VA_ARGS__)

#define D_SUCCESS(...) \
    Logger::Succsess(__VA_ARGS__)

#else
#define D_PRINT(...)

#define D_WARN(...)

#define D_ERR(...)

#define D_SUCCESS(...)
#endif
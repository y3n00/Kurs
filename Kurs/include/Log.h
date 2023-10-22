#pragma once
#include <iostream>
#include <utility>

namespace Logger {
template <typename... Args>
static inline void Error(Args&&... args) noexcept {
  std::cout << "\x1B[31m";
  ((std::cout << std::forward<Args>(args) << ' '), ...);
  std::cout << "\033[0m" << std::endl;
}

template <typename... Args>
static inline void Succsess(Args&&... args) noexcept {
  std::cout << "\x1B[32m";
  ((std::cout << std::forward<Args>(args) << ' '), ...);
  std::cout << "\033[0m" << std::endl;
}

template <typename... Args>
static inline void Warning(Args&&... args) noexcept {
  std::cout << "\x1B[33m";
  ((std::cout << std::forward<Args>(args) << ' '), ...);
  std::cout << "\033[0m" << std::endl;
}

template <typename... Args>
static inline void Print(Args&&... args) noexcept {
  ((std::cout << std::forward<Args>(args) << ' '), ...) << std::endl;
}
}  // namespace Logger

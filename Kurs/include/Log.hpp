#pragma once
#include <iostream>

namespace Logger {
  static inline bool new_line_enabled = true;
  template <typename... Args>
  static inline void Error(Args&&... args) noexcept {
    std::cout << std::boolalpha << "\x1B[31m";
    ((std::cout << std::forward<Args>(args) << ' '), ...);
    std::cout << "\033[0m";
    if (new_line_enabled) std::cout << '\n';
  }

  template <typename... Args>
  static inline void Success(Args&&... args) noexcept {
    std::cout << std::boolalpha << "\x1B[32m";
    ((std::cout << std::forward<Args>(args) << ' '), ...);
    std::cout << "\033[0m";
    if (new_line_enabled) std::cout << '\n';
  }

  template <typename... Args>
  static inline void Warning(Args&&... args) noexcept {
    std::cout << std::boolalpha << "\x1B[33m";
    ((std::cout << std::forward<Args>(args) << ' '), ...);
    std::cout << "\033[0m";
    if (new_line_enabled) std::cout << '\n';
  }

  template <typename... Args>
  static inline void Print(Args&&... args) noexcept {
    std::cout << std::boolalpha;
    ((std::cout << std::forward<Args>(args) << ' '), ...);
    if (new_line_enabled) std::cout << '\n';
  }
}  // namespace Logger
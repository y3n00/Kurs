#pragma once
#define NOMINMAX
#include <corecrt_wstdio.h>
#include <windows.h>

#include <concepts>
#include <string>
#include <string_view>
namespace Console {
template <std::integral T>
struct SZ {
    T width, height;
    SZ() = default;
    SZ(T&& w, T&& h) : width{w}, height{h} {}
    SZ(const T& w, const T& h) : width{w}, height{h} {}
};

[[nodiscard]] inline auto getSizeByPixels() {
    RECT tempRect{};
    GetWindowRect(GetConsoleWindow(), &tempRect);
    return SZ{tempRect.right - tempRect.left, tempRect.bottom - tempRect.top};
}

[[nodiscard]] inline auto getSizeByChars() {
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return SZ<uint16_t>{csbi.srWindow.Right - csbi.srWindow.Left + 1, csbi.srWindow.Bottom - csbi.srWindow.Top + 1};
}

inline void setSizeByPixels(uint16_t w, uint16_t h) {
    RECT tempRect{};
    GetWindowRect(GetConsoleWindow(), &tempRect);
    MoveWindow(GetConsoleWindow(), tempRect.left, tempRect.top, w, h, TRUE);
}

inline void setSizeByPixels(const SZ<uint16_t>& newSize) {
    const auto& [w, h] = newSize;
    setSizeByPixels(w, h);
}

[[nodiscard]] inline auto getFontSize() {
    auto cfi = CONSOLE_FONT_INFOEX{.cbSize = sizeof(CONSOLE_FONT_INFOEX)};
    GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &cfi);
    return cfi.dwFontSize.X;
}

inline void setFont(int newFontSize, const wchar_t* newFont = L"Consolas") {
    auto cfi = CONSOLE_FONT_INFOEX{.cbSize = sizeof(CONSOLE_FONT_INFOEX)};
    GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &cfi);

    swprintf_s(cfi.FaceName, newFont);
    cfi.dwFontSize.Y = newFontSize;

    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &cfi);
}

[[nodiscard]] inline auto getWindowSizeByChars() {
    auto csbi = CONSOLE_SCREEN_BUFFER_INFO{};
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return SZ{csbi.srWindow.Right - csbi.srWindow.Left + 1, csbi.srWindow.Bottom - csbi.srWindow.Top + 1};
}

inline void showCursor(bool show) {
    CONSOLE_CURSOR_INFO structCursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &structCursorInfo);
    structCursorInfo.bVisible = show;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &structCursorInfo);
}

inline void setCursorPos(uint8_t newX, uint8_t newY) {
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {newX, newY});
}

inline void setTitle(std::string_view newTitle) {
    const auto wStr = std::wstring{newTitle.begin(), newTitle.end()}.c_str();
    SetConsoleTitle(wStr);
}

inline void Configure(std::string_view title, const SZ<uint16_t>& size) {
    DWORD dwMode{};
    GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), dwMode);

    setTitle(title);
    setSizeByPixels(size)
}
}  // namespace Console
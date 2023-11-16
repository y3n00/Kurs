#pragma once
#define NOMINMAX
#include <corecrt_wstdio.h>
#include <windows.h>

#include <concepts>
#include <string>
#include <string_view>

#define GETTER [[nodiscard]] inline decltype(auto)
#define GETTER_T(Ty) [[nodiscard]] inline Ty
#define SETTER inline void

namespace Console {
template <std::integral T>
struct SZ {
    T width, height;
    constexpr SZ() : width{}, height{} {}
    constexpr SZ(T&& w, T&& h) : width{w}, height{h} {}
    constexpr SZ(const T& w, const T& h) : width{w}, height{h} {}
};

GETTER getSizeByPixels() {
    RECT tempRect{};
    GetWindowRect(GetConsoleWindow(), &tempRect);
    return SZ{
        tempRect.right - tempRect.left,
        tempRect.bottom - tempRect.top};
}

GETTER getSizeByChars() {
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return SZ{
        csbi.srWindow.Right - csbi.srWindow.Left + 1,
        csbi.srWindow.Bottom - csbi.srWindow.Top + 1};
}

SETTER setSizeByPixels(const SZ<uint16_t>& newSize) {
    const auto& [w, h] = newSize;
    RECT tempRect{};
    GetWindowRect(GetConsoleWindow(), &tempRect);
    MoveWindow(GetConsoleWindow(), tempRect.left, tempRect.top, w, h, TRUE);
}

SETTER setSizeByChars(const SZ<int16_t>& newSize) {
    const auto& [w, h] = newSize;
    const COORD coord{w, h};
    const SMALL_RECT rect{0, 0, w - 1, h - 1};
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coord);  // Set Buffer Size
    SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &rect);  // Set Window Size
}

GETTER getFontSize() {
    auto cfi = CONSOLE_FONT_INFOEX{.cbSize = sizeof(CONSOLE_FONT_INFOEX)};
    GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &cfi);
    return cfi.dwFontSize.X;
}

SETTER setFont(int16_t newFontSize, const wchar_t* newFont = L"Consolas") {
    auto cfi = CONSOLE_FONT_INFOEX{.cbSize = sizeof(CONSOLE_FONT_INFOEX)};
    GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &cfi);

    swprintf_s(cfi.FaceName, newFont);
    cfi.dwFontSize.Y = newFontSize;

    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &cfi);
}

SETTER toggleCursor(bool show) {
    CONSOLE_CURSOR_INFO structCursorInfo;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &structCursorInfo);
    structCursorInfo.bVisible = show;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &structCursorInfo);
}

SETTER setCursorPos(const Console::SZ<int16_t>& place) {
    const auto& [newX, newY] = place;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {newX, newY});
}

GETTER getWindowInfo() {
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo{};
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleInfo);
    return consoleInfo;
}

SETTER setTitle(std::string_view newTitle) {
    SetConsoleTitle(std::wstring{newTitle.begin(), newTitle.end()}.c_str());
}

SETTER configure(std::string_view title, const SZ<uint16_t>& size) {
    DWORD dwMode{};
    GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &dwMode);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), (dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING));

    setTitle(title);
    setSizeByPixels(size);
}

SETTER putStr(std::string_view str, const SZ<int16_t>& place) {
    setCursorPos(place);
    std::cout << str;
}

GETTER getCursorPosition() {
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo{};
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleInfo);
    const auto [X, Y] = consoleInfo.dwCursorPosition;
    return SZ<int16_t>{X, Y};
}
}  // namespace Console
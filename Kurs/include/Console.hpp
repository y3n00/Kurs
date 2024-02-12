#pragma once
#define NOMINMAX
#include <Windows.h>

#include <concepts>
#include <format>
#include <iostream>
#include <string>

#define GETTER [[nodiscard]] inline decltype(auto)
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
        RECT rect{};
        GetWindowRect(GetConsoleWindow(), &rect);
        return SZ{rect.right - rect.left, rect.bottom - rect.top};
    }

    GETTER getSizeByChars() {
        auto&& handle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        GetConsoleScreenBufferInfo(handle, &csbi);
        return SZ{csbi.srWindow.Right - csbi.srWindow.Left + 1,
                  csbi.srWindow.Bottom - csbi.srWindow.Top + 1};
    }

    SETTER setSizeByPixels(const SZ<uint16_t>& newSize) {
        const auto& [W, H] = newSize;
        RECT rect{};
        GetWindowRect(GetConsoleWindow(), &rect);
        MoveWindow(GetConsoleWindow(), rect.left, rect.top, W, H, TRUE);
    }

    SETTER setSizeByChars(const SZ<int16_t>& newSize) {
        auto&& handle = GetStdHandle(STD_OUTPUT_HANDLE);
        auto&& [w, h] = newSize;
        COORD coord{w, h};
        SMALL_RECT Rect{0, 0, w - 1, h - 1};
        SetConsoleScreenBufferSize(handle, coord);
        SetConsoleWindowInfo(handle, TRUE, &Rect);
    }

    GETTER getFontSize() {
        auto cfi = CONSOLE_FONT_INFOEX{.cbSize = sizeof(CONSOLE_FONT_INFOEX)};
        GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &cfi);
        return cfi.dwFontSize.X;
    }

    SETTER setFont(int16_t newFontSize, const wchar_t* newFont = L"Consolas") {
        auto&& handle = GetStdHandle(STD_OUTPUT_HANDLE);
        auto cfi = CONSOLE_FONT_INFOEX{.cbSize = sizeof(CONSOLE_FONT_INFOEX)};
        GetCurrentConsoleFontEx(handle, NULL, &cfi);
        swprintf_s(cfi.FaceName, newFont);
        cfi.dwFontSize.Y = newFontSize;
        SetCurrentConsoleFontEx(handle, NULL, &cfi);
    }

    SETTER toggleCursor() {
        CONSOLE_CURSOR_INFO cci;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);
        cci.bVisible = !cci.bVisible;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);
    }

    SETTER setCursorPos(const Console::SZ<int16_t>& place) {
        auto&& handle = GetStdHandle(STD_OUTPUT_HANDLE);
        const auto& [newX, newY] = place;
        SetConsoleCursorPosition(handle, {newX, newY});
    }

    GETTER getWindowInfo() {
        auto&& handle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        GetConsoleScreenBufferInfo(handle, &csbi);
        return csbi;
    }

    SETTER setTitle(std::string_view newTitle) {
        SetConsoleTitle(std::wstring{newTitle.begin(), newTitle.end()}.c_str());
    }

    SETTER configure(std::string_view title, const SZ<uint16_t>& size) {
        auto&& handle = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD m{};
        GetConsoleMode(handle, &m);
        SetConsoleMode(handle, (m |= ENABLE_VIRTUAL_TERMINAL_PROCESSING));
        setTitle(title);
        setSizeByPixels(size);
    }

    SETTER putStr(std::string_view str, const SZ<int16_t>& place) {
        setCursorPos(place);
        std::cout << str;
    }

    GETTER getCursorPosition() {
        auto&& handle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        GetConsoleScreenBufferInfo(handle, &csbi);
        const auto& [X, Y] = csbi.dwCursorPosition;
        return SZ<int16_t>{X, Y};
    }
}  // namespace Console
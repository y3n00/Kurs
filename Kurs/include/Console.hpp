#pragma once
#define NOMINMAX
#include <corecrt_wstdio.h>
#include <windows.h>

#include <concepts>
#include <iostream>
#include <string>
#include <string_view>

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
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
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
        const auto& [W, H] = newSize;
        const COORD coord{W, H};
        const SMALL_RECT rect{0, 0, W - 1, H - 1};
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

    SETTER toggleCursor() {
        CONSOLE_CURSOR_INFO cci;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);
        cci.bVisible = !cci.bVisible;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);
    }

    SETTER setCursorPos(const Console::SZ<int16_t>& place) {
        const auto& [newX, newY] = place;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {newX, newY});
    }

    GETTER getWindowInfo() {
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        return csbi;
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
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        const auto& [X, Y] = csbi.dwCursorPosition;
        return SZ<int16_t>{X, Y};
    }
}  // namespace Console
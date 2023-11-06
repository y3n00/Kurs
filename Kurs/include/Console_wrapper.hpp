#pragma once
#define NOMINMAX
#include <conio.h>

enum Keys : int {
    BACKSPACE = 8,
    ENTER = 13,
    ESCAPE = 27,
    SPACE = 32,
    _0 = 48,
    _1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,
    _9,
    UP_ARR = 72,
    LEFT_ARR = 75,
    RIGHT_ARR = 77,
    DOWN_ARR = 80,
};

#include <algorithm>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include "Console.hpp"
#include "Log.hpp"

class Console_wrapper {
   private:
    static constexpr inline uint8_t BORDER_PADDING = 2;

    [[nodiscard]] static inline auto get_lines(const std::string& str) {
        std::vector<std::string> strs;
        strs.reserve(50);
        std::stringstream sstr{str};

        std::string buf;
        while (std::getline(sstr, buf))
            strs.emplace_back(buf);

        return strs;
    }

    static auto put_str(std::string_view str, const Console::SZ<int16_t>& place) {
        Console::setCursorPos(place);
        std::cout << str;
    }

    static inline void new_line() {
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getWindowInfo().dwCursorPosition;  // current cursor position

        const auto CLAMPED = std::clamp<int16_t>(CURSOR_Y + 1, CURSOR_Y, CON_HEIGHT - BORDER_PADDING);
        Console::setCursorPos({1, CLAMPED});  // move cursor at the beginning of the next line
    }

   public:
    static void draw_frame(char vert, char hor) {
        static constexpr Console::SZ<int16_t> DEFAULT_PLACEHOLDER{1, 1};

        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto HOR_BORDER = std::string(CON_WIDTH, hor);
        const auto VERT_BORDER = vert + std::string(CON_WIDTH - BORDER_PADDING, ' ') + vert;

        put_str(HOR_BORDER, {0, 0});                        // top line
        put_str(HOR_BORDER, {0, int16_t(CON_HEIGHT - 1)});  // bottom line

        for (int16_t i = 1; i < CON_HEIGHT - 1; i++)  // vertical borders, -1 for bottom border
            put_str(VERT_BORDER, {0, i});             //
        Console::setCursorPos(DEFAULT_PLACEHOLDER);
    }

    static void write(std::string_view msg) {
        const auto MSG_LINES = get_lines(std::string(msg));
        if (MSG_LINES.empty())
            return;

        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getWindowInfo().dwCursorPosition;  // current cursor position

        const int32_t ACTUAL_CON_WIDTH = CON_WIDTH - BORDER_PADDING;

        for (const auto& [idx, line] : MSG_LINES | std::views::enumerate) {
            const Console::SZ PLACE(CURSOR_X, int16_t(CURSOR_Y + idx));
            const size_t EXTRA_SPACE_HOR = ACTUAL_CON_WIDTH - (CURSOR_X + line.length());

            if (EXTRA_SPACE_HOR >= 0) {  // if line fully fits
                put_str(line, PLACE);
            } else {
                const size_t FITTED_SIZE = msg.length() + EXTRA_SPACE_HOR;  // EXTRA_SPACE is negative
                put_str(line.substr(0, FITTED_SIZE - 3) + "...", PLACE);    // -3 for "..."
            }
        }
    }

    static void writeln(std::string_view msg) {
        write(msg);  // write in current cursor pos
        new_line();
    }

    template <typename T>
    static T get_input() {
        const auto [X, Y] = Console::getWindowInfo().dwCursorPosition;
        T buf{};
        if constexpr (std::is_same_v<T, std::string>)
            std::getline(std::cin, buf);
        else
            (std::cin >> buf).get();
        Console::setCursorPos({X, Y});
        new_line();
        return buf;
    }
};

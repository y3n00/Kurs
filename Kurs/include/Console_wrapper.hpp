#pragma once
#define NOMINMAX
/*
#include <conio.h>
enum Keys : int {
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
*/

#include <algorithm>
#include <iostream>
#include <sstream>

#include "Console.hpp"
#include "Log.hpp"

class Console_wrapper {
   private:
    static constexpr inline uint8_t BORDER_PADDING = 2;

    [[nodiscard]] static inline auto get_lines(const std::string& str) {
        std::vector<std::string> strs;
        strs.reserve(50);
        std::stringstream sstr{str};
        size_t counter{};

        std::string buf;
        while (std::getline(sstr, buf))
            strs.emplace_back(buf);

        return strs;
    }

    static auto put_str(std::string_view str, const Console::SZ<int16_t>& place) {
        Console::setCursorPos(place);
        std::cout << str;
    }

   public:
    static auto draw_frame(char vert, char hor) {
        static constexpr Console::SZ<int16_t> DEFAULT_PLACEHOLDER{1, 1};
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto hor_border = std::string(CON_WIDTH, hor);
        const auto vert_border = vert + std::string(CON_WIDTH - BORDER_PADDING, ' ') + vert;

        put_str(hor_border, {0, 0});                        // top line
        put_str(hor_border, {0, int16_t(CON_HEIGHT - 1)});  // bottom line

        for (int16_t i = 1; i < CON_HEIGHT - 1; i++)  // verical borders, 1 for bottom border
            put_str(vert_border, {0, i});             //
        Console::setCursorPos(DEFAULT_PLACEHOLDER);
    }

    static auto write(std::string_view msg) {
        const auto MSG_LINES = get_lines(std::string(msg));
        if (MSG_LINES.empty())
            return;

        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();               // current console size
        const auto [CURSOR_X, CURSOR_Y] = Console::getWindowInfo().dwCursorPosition;  // current cursor position

        const int32_t ACTUAL_CON_WIDTH = CON_WIDTH - BORDER_PADDING;

        for (const auto& [idx, line] : MSG_LINES | std::views::enumerate) {
            const int32_t EXTRA_SPACE_HOR = (ACTUAL_CON_WIDTH - (CURSOR_X + line.length()));
            if (EXTRA_SPACE_HOR >= 0) {  // if line fully fits
                put_str(line, { int16_t(CURSOR_X + idx), int16_t(CURSOR_Y + 1) });
            } else {
                const size_t FITTED_SIZE = msg.length() + EXTRA_SPACE_HOR;                         // EXTRA_SPACE is negative
                put_str(line.substr(0, FITTED_SIZE - 3) + "...", { int16_t(CURSOR_X + idx), int16_t(CURSOR_Y + 1) });  // -3 for "..."
            }
        }
    }

    static auto writeln(std::string_view msg) {
        write(msg);  // write in current cursor pos

        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();               // current console size
        const auto [CURSOR_X, CURSOR_Y] = Console::getWindowInfo().dwCursorPosition;  // current cursor position

        const auto clamped = std::clamp<int16_t>(CURSOR_Y + 1, CURSOR_Y, CON_HEIGHT - BORDER_PADDING);
        Console::setCursorPos({1, clamped});  // move cursor at the begining of the next line
    }
};

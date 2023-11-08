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
    static inline char vert_symb, hor_symb;
    static constexpr inline uint8_t BORDER_PADDING = 2;

    [[nodiscard]] static inline auto split_lines(std::string_view str) {
        std::vector<std::string> strs;
        strs.reserve(50);
        std::stringstream sstr{str.data()};

        std::string buf;
        while (std::getline(sstr, buf))
            strs.emplace_back(buf);

        return strs;
    }

    static inline void put_str(std::string_view str, const Console::SZ<int16_t>& place) {
        Console::setCursorPos(place);
        std::cout << str;
    }

    static inline void new_line() {
        const uint16_t CON_HEIGHT = Console::getSizeByChars().height;
        const uint16_t CURRENT_Y = Console::getWindowInfo().dwCursorPosition.Y;

        const auto Y_NEXT = std::clamp<int16_t>(CURRENT_Y + 1, CURRENT_Y, CON_HEIGHT - BORDER_PADDING);
        Console::setCursorPos({1, Y_NEXT});  // move cursor at the beginning of the next line
    }

   public:
    static void set_frame_border(char vert_, char hor_) {
        vert_symb = vert_, hor_symb = hor_;
    }

    static void draw_frame() {
        static constexpr Console::SZ<int16_t> DEFAULT_PLACE{1, 1};

        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto HOR_BORDER = std::string(CON_WIDTH, hor_symb);
        const auto VERT_BORDER = vert_symb + std::string(CON_WIDTH - BORDER_PADDING, ' ') + vert_symb;

        put_str(HOR_BORDER, {0, 0});                        // top line
        put_str(HOR_BORDER, {0, int16_t(CON_HEIGHT - 1)});  // bottom line

        for (int16_t i = 1; i < CON_HEIGHT - 1; i++)  // vertical borders, -1 for bottom border
            put_str(VERT_BORDER, {0, i});             //
        Console::setCursorPos(DEFAULT_PLACE);
    }

    static void write(std::string_view msg) {
        const auto MSG_BY_LINES = split_lines(msg);
        if (MSG_BY_LINES.empty())
            return;

        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getWindowInfo().dwCursorPosition;  // current cursor position
        const int32_t ACTUAL_CON_WIDTH = CON_WIDTH - BORDER_PADDING;
        const int32_t ACTUAL_CON_HEIGHT = CON_HEIGHT - BORDER_PADDING;

        for (const auto& [idx, line] : MSG_BY_LINES | std::views::enumerate) {
            const Console::SZ PLACE(CURSOR_X, int16_t(CURSOR_Y + idx));
            const size_t FITTED_SIZE = ACTUAL_CON_WIDTH - (CURSOR_X + line.length());

            if (FITTED_SIZE >= 0) {  // if line fully fits
                put_str(line, PLACE);
            } else {
                const size_t FITTED_SIZE = msg.length() + FITTED_SIZE;    // FITTED_SIZE is negative
                put_str(line.substr(0, FITTED_SIZE - 3) + "...", PLACE);  // -3 for "..."
            }
        }
    }

    static inline void writeln(std::string_view msg) {
        write(msg);
        new_line();
    }

    template <typename T>
    [[nodiscard]] static T get_input() {
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getWindowInfo().dwCursorPosition;

        put_str(std::string(CON_WIDTH - 2, hor_symb), {1, int16_t(CON_HEIGHT - 3)});
        put_str("Ввод: ", {1, int16_t(CON_HEIGHT - 2)});

        T buf{};
        if constexpr (std::is_same_v<T, std::string>)
            std::getline(std::cin, buf);
        else
            (std::cin >> buf).get();

        std::cin.clear();
        Console::setCursorPos({CURSOR_X, CURSOR_Y});
        return buf;
    }

    static inline void clear_screen() {
        system("cls");
        draw_frame();
    }
};

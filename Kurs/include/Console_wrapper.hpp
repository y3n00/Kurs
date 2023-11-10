#pragma once
#define NOMINMAX
#include <conio.h>

enum Keys : int16_t {
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
#include <functional>
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

    static inline void new_line() {
        const auto CON_HEIGHT = Console::getSizeByChars().height;
        const auto CURRENT_Y = Console::getWindowInfo().dwCursorPosition.Y;

        const auto Y_NEXT = std::clamp<int16_t>(CURRENT_Y + 1, CURRENT_Y, CON_HEIGHT - BORDER_PADDING);
        Console::setCursorPos({1, Y_NEXT});  // move cursor at the beginning of the next line
    }

   public:
    static inline void write_vec(const std::vector<std::string>& vec) {
        if (vec.empty())
            return;
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getWindowInfo().dwCursorPosition;
        const uint16_t ACTUAL_WIDTH = CON_WIDTH - BORDER_PADDING, ACTUAL_HEIGHT = CON_HEIGHT - BORDER_PADDING;
        const uint16_t FREE_VERT_SPACE = ACTUAL_HEIGHT - CURSOR_Y + 1;

        const int32_t SIZE_DIFF = FREE_VERT_SPACE - vec.size();
        const bool MSG_IS_BIGGER = SIZE_DIFF < 0;
        const uint32_t MAX_IDX = MSG_IS_BIGGER ? FREE_VERT_SPACE - 1 : vec.size();

        for (size_t i = 0; i < MAX_IDX; ++i) {
            const auto& line = vec[i];
            const Console::SZ<int16_t> PLACE{int16_t(1), int16_t(CURSOR_Y + i)};
            const int32_t FITTED_PART = ACTUAL_WIDTH - (CURSOR_X + line.length());

            if (FITTED_PART >= 0) {  // if line fully fits
                Console::putStr(line, PLACE);
            } else {
                const auto FITTED_SIZE = -FITTED_PART + line.length();            // FITTED_SIZE is negative
                Console::putStr(line.substr(0, FITTED_SIZE - 3) + "...", PLACE);  // -3 for "..."
            }
        }
        if (MSG_IS_BIGGER) {
            new_line();
            write(std::format("Ещё {} строк...", -SIZE_DIFF + 1));  // SIZE_DIFF is negative
        }
    }

    static inline void set_frame_border(char vert, char hor) {
        vert_symb = vert, hor_symb = hor;
    }

    static void draw_frame() {
        static constexpr Console::SZ<int16_t> DEFAULT_PLACE{1, 1};

        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto HOR_BORDER = std::string(CON_WIDTH, hor_symb);
        const auto VERT_BORDER = vert_symb + std::string(CON_WIDTH - BORDER_PADDING, ' ') + vert_symb;

        Console::putStr(HOR_BORDER, {0, 0});                        // top line
        Console::putStr(HOR_BORDER, {0, int16_t(CON_HEIGHT - 1)});  // bottom line

        for (int16_t i = 1; i < CON_HEIGHT - 1; i++)  // vertical borders, -1 for bottom border
            Console::putStr(VERT_BORDER, {0, i});     //
        Console::setCursorPos(DEFAULT_PLACE);
    }

    static void write(std::string_view msg) {
        write_vec(split_lines(msg));
    }

    static inline void writeln(std::string_view msg) {
        write(msg);
        new_line();
    }

    template <typename T>
    [[nodiscard]] constexpr static T get_input() {
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getWindowInfo().dwCursorPosition;

        Console::putStr(std::string(CON_WIDTH - 2, hor_symb), {1, int16_t(CON_HEIGHT - 3)});
        Console::putStr("Ввод: ", {1, int16_t(CON_HEIGHT - 2)});

        T buf{};
        if constexpr (std::is_same_v<T, std::string>)
            std::getline(std::cin, buf);
        else
            (std::cin >> buf).get();

        std::cin.clear();
        Console::setCursorPos({CURSOR_X, CURSOR_Y});
        return buf;
    }

    template <typename T, typename U>
    static auto vector_selection(const std::vector<T>& vec, std::function<U(const T& t)>&& get_value) {
        draw_frame();
        if (vec.empty()) {
            writeln("vector is empty, nothing to show");
            return -1;
        }
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getWindowInfo().dwCursorPosition;

        const int32_t ACTUAL_HEIGHT = CON_HEIGHT - BORDER_PADDING;
        const auto ENUMED_VEC = vec | std::views::enumerate;
        const int32_t MIN_IDX = 0, MAX_IDX = vec.size() - 1;
        const auto SCOPED_CLAMP = std::bind(std::clamp<int32_t>, std::placeholders::_1, MIN_IDX, MAX_IDX);

        int32_t pressed_key = 0, current_selection = 0;
        do {
            clear_border();
            switch (pressed_key) {
                case Keys::DOWN_ARR:
                    current_selection = SCOPED_CLAMP(++current_selection);  // stop at last
                    break;
                case Keys::UP_ARR:
                    current_selection = SCOPED_CLAMP(--current_selection);  // stop at first
                    break;
            }
            for (auto&& [idx, data] : ENUMED_VEC) {
                const std::string FMT = std::format("{0: >2}) {1}", idx, get_value(data));
                if (current_selection == idx)
                    writeln('>' + FMT);
                else
                    writeln(FMT);
            }
        } while ((pressed_key = _getch()) != Keys::ENTER);
        clear_border();
        return current_selection;
    }

    static inline void clear_border() {
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const std::string empty_space(CON_WIDTH - BORDER_PADDING, ' ');
        for (size_t i = 1; i <= CON_HEIGHT - BORDER_PADDING; i++)
            Console::putStr(empty_space, {1, int16_t(i)});
        Console::setCursorPos({1, 1});
    }
};

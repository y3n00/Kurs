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

#define EMPTY_CHECK(RANGE)                  \
    if (RANGE.empty()) {                    \
        writeln("Вектор пуст ¯\\_(ツ)_/¯"); \
        return;                             \
    }

[[nodiscard]] inline auto split_lines(std::string_view str) {
    return str | std::views::split('\n') | std::ranges::to<std::vector<std::string>>();
}

class Console_wrapper {
   private:
    static inline char vert_symb, hor_symb;
    static constexpr inline uint8_t BORDER_PADDING = 2;

   public:
    static inline void new_line() {
        const auto CON_HEIGHT = Console::getSizeByChars().height;
        const auto CURRENT_Y = Console::getCursorPosition().height;

        const auto Y_NEXT = std::clamp<int16_t>(CURRENT_Y + 1, CURRENT_Y, CON_HEIGHT - BORDER_PADDING);
        Console::setCursorPos({1, Y_NEXT});
    }

    static void write_vec(const std::vector<std::string>& vec, bool enumerate = true) {
        EMPTY_CHECK(vec);
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getCursorPosition();
        const uint16_t ACTUAL_HEIGHT = CON_HEIGHT - BORDER_PADDING;

        const int32_t VERT_SIZE_DIFF = ACTUAL_HEIGHT - vec.size();
        const bool VEC_IS_BIGGER = VERT_SIZE_DIFF < 0;
        const uint32_t MAX_IDX = VEC_IS_BIGGER ? ACTUAL_HEIGHT - 1 : vec.size();

        auto&& ENUMED = vec | std::views::enumerate;
        auto&& print_subrange = [&](auto&& enumed_sub_range) -> void {
            for (auto&& [idx, data] : enumed_sub_range)
                writeln((enumerate ? std::format("{0}", data) : std::format("{0: >2}) {1}", idx + 1, data)));
        };

        if (!VEC_IS_BIGGER)
            print_subrange(ENUMED);
        else {
            auto&& CHUNKED = ENUMED | std::views::chunk(MAX_IDX);
            const size_t N_PAGES = CHUNKED.size();
            auto&& PAGE_CLAMP = std::bind(std::clamp<uint16_t>, std::placeholders::_1, 0, N_PAGES - 1);
            int32_t current_page = 0, pressed_key = 0;
            do {
                clear_border();
                if (pressed_key == Keys::LEFT_ARR)
                    current_page = PAGE_CLAMP(--current_page);
                else if (pressed_key == Keys::RIGHT_ARR)
                    current_page = PAGE_CLAMP(++current_page);
                print_subrange(CHUNKED[current_page]);
                write(std::format("{} страница из {}", current_page + 1, N_PAGES));
            } while ((pressed_key = _getch()) != Keys::ENTER);
        }
    }

    static inline void set_frame_border(char vert, char hor) {
        vert_symb = vert, hor_symb = hor;
    }

    static void draw_frame() {
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const std::string HOR_BORDER = std::string(CON_WIDTH, hor_symb);
        const std::string VERT_BORDER = vert_symb + std::string(CON_WIDTH - BORDER_PADDING, ' ') + vert_symb;
        const auto MAX_Y_POS = int16_t(CON_HEIGHT - 1);  //-1 for 0-indexed count

        Console::putStr(HOR_BORDER, {0, 0});          // top line
        Console::putStr(HOR_BORDER, {0, MAX_Y_POS});  // bottom line

        for (int16_t i = 1; i < MAX_Y_POS; i++)    // vertical borders
            Console::putStr(VERT_BORDER, {0, i});  //
        Console::setCursorPos({1, 1});
    }

    static void write(std::string_view str) {
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getCursorPosition();
        const int16_t ACTUAL_WIDTH = CON_WIDTH - BORDER_PADDING;
        const int32_t FITTED_PART = ACTUAL_WIDTH - (CURSOR_X + str.length());

        if (FITTED_PART >= 0)  // if line horizontaly fits
            writeln(str);
        else {
            const auto FITTED_SIZE = -FITTED_PART + str.length();          // FITTED_SIZE is negative
            writeln(std::string{str}.substr(0, FITTED_SIZE - 3) + "...");  // -3 for "..."
        }
    }

    static inline void writeln(std::string_view msg) {
        write(msg);
        new_line();
    }

    template <typename T>
    [[nodiscard]] constexpr static T get_input() {
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getCursorPosition();
        Console::putStr(std::string(CON_WIDTH - BORDER_PADDING, hor_symb), {1, int16_t(CON_HEIGHT - 3)});
        Console::putStr("Ввод: ", {1, int16_t(CON_HEIGHT - BORDER_PADDING)});

        T buf{};
        if constexpr (std::is_same_v<T, std::string>)
            std::getline(std::cin, buf);
        else
            (std::cin >> buf).get();
        std::cin.clear();
        Console::setCursorPos({CURSOR_X, CURSOR_Y});  // restore cursor position
        return buf;
    }

    template <typename T>  // TODO PAGES
    [[nodiscard]] static uint32_t vec_selection(const std::vector<T>& vec, std::string (*get_value)(const T& obj) = nullptr) {
        EMPTY_CHECK(vec);
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getCursorPosition();
        const int32_t ACTUAL_HEIGHT = CON_HEIGHT - BORDER_PADDING;
        const int32_t MIN_IDX = 0, MAX_IDX = vec.size() - 1;
        auto&& SCOPED_CLAMP = std::bind(std::clamp<int32_t>, std::placeholders::_1, MIN_IDX, MAX_IDX);

        int32_t pressed_key = 0, current_selection = 0;
        do {
            clear_border();
            if (pressed_key == Keys::DOWN_ARR)
                current_selection = SCOPED_CLAMP(--current_selection);
            else if (pressed_key == Keys::UP_ARR)
                current_selection = SCOPED_CLAMP(++current_selection);
            for (auto&& [idx, data] : vec | std::views::enumerate) {
                std::string str_buf;
                if constexpr (std::is_same_v<T, std::string>)
                    str_buf = std::format("{0: >2}) {1}", idx, data);
                else
                    str_buf = std::format("{0: >2}) {1}", idx, get_value(data));
                current_selection == idx ? writeln('>' + str_buf) : writeln(str_buf);
            }
        } while ((pressed_key = _getch()) != Keys::ENTER);
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

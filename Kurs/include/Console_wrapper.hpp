#pragma once
#define NOMINMAX
#include <conio.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include "Console.hpp"
#include "Log.hpp"

constexpr inline auto ENUMED_ELEM_FMT = "{0: >2}) {1}";

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

    static void vec_print(const std::vector<std::string>& vec, bool enumerate = true) {
        if (vec.empty()) {
            Logger::Error("Вектор пуст!");
            return;
        }
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getCursorPosition();
        const uint16_t ACTUAL_HEIGHT = CON_HEIGHT - BORDER_PADDING;

        const int16_t VERT_SIZE_DIFF = ACTUAL_HEIGHT - vec.size();
        const bool VEC_IS_BIGGER = VERT_SIZE_DIFF < 0;
        const uint16_t MAX_IDX = VEC_IS_BIGGER ? ACTUAL_HEIGHT - 1 : vec.size();

        auto&& ENUMED = vec | std::views::enumerate;
        auto&& print_subrange = [enumerate](auto&& enumed_subrange) -> void {
            for (auto&& [idx, data] : enumed_subrange)
                writeln((enumerate ? std::format(ENUMED_ELEM_FMT, idx + 1, data) : std::format("{}", data)));
        };

        if (!VEC_IS_BIGGER)
            print_subrange(ENUMED);
        else {
            auto&& CHUNKED = ENUMED | std::views::chunk(MAX_IDX);
            const uint16_t N_PAGES = CHUNKED.size();
            auto&& PAGE_CLAMP = std::bind(std::clamp<int16_t>, std::placeholders::_1, 0, N_PAGES - 1);
            int16_t current_page = 0, pressed_key = 0;
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
        const std::string HOR_BORDER(CON_WIDTH, hor_symb);
        const std::string VERT_BORDER = vert_symb + std::string(CON_WIDTH - BORDER_PADDING, ' ') + vert_symb;
        const auto MAX_Y_POS = int16_t(CON_HEIGHT - 1);  // -1 for 0-indexed count

        Console::putStr(HOR_BORDER, {0, 0});          // top line
        Console::putStr(HOR_BORDER, {0, MAX_Y_POS});  // bottom line
        for (int16_t i = 1; i < MAX_Y_POS; i++)       // vertical borders
            Console::putStr(VERT_BORDER, {0, i});     //
        Console::setCursorPos({1, 1});
    }

    static void write(std::string_view str) {
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getCursorPosition();
        const int16_t ACTUAL_WIDTH = CON_WIDTH - BORDER_PADDING;
        const int32_t FITTED_PART = ACTUAL_WIDTH - (CURSOR_X + str.length());

        if (FITTED_PART >= 0)  // if line fits horizontaly
            std::cout << str;
        else {
            const auto FITTED_SIZE = FITTED_PART + str.length();  // FITTED_SIZE is negative
            std::cout << str.substr(0, FITTED_SIZE - 3) << "...";  // -3 for "..."
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

    [[nodiscard]] static std::string vec_selection(const std::vector<std::string>& vec) {
        if (vec.empty()) {
            Logger::Error("Вектор пуст!");
            return {};
        }
        int32_t return_value = -1;
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getCursorPosition();
        const uint16_t ACTUAL_HEIGHT = CON_HEIGHT - BORDER_PADDING;

        const int32_t VERT_SIZE_DIFF = ACTUAL_HEIGHT - vec.size();
        const bool VEC_IS_BIGGER = VERT_SIZE_DIFF < 0;
        const uint32_t MAX_IDX = VEC_IS_BIGGER ? ACTUAL_HEIGHT - 1 : vec.size();
        auto&& ENUMED = vec | std::views::enumerate;

        const auto subrange_selection = [](auto&& subrange) -> int32_t {
            clear_border();
            const auto IDX_MIN = std::get<0>(subrange.front());  // min index
            const auto IDX_MAX = std::get<0>(subrange.back());   // max index
            auto&& in_page_clamp = std::bind(std::clamp<int32_t>, std::placeholders::_1, IDX_MIN, IDX_MAX);
            int scoped_idx = IDX_MIN, pressed_key = 0;
            do {
                clear_border();
                if (pressed_key == Keys::DOWN_ARR)
                    scoped_idx = in_page_clamp(++scoped_idx);
                else if (pressed_key == Keys::UP_ARR)
                    scoped_idx = in_page_clamp(--scoped_idx);
                else if (pressed_key == Keys::ENTER)
                    return scoped_idx;
                else if (pressed_key == Keys::ESCAPE)
                    return -1;

                for (auto&& [idx, elem] : subrange) {
                    const auto fmt = std::format(ENUMED_ELEM_FMT, idx, elem);
                    scoped_idx == idx ? writeln('>' + fmt) : writeln(fmt);
                }
            } while (pressed_key = _getch());
        };

        if (!VEC_IS_BIGGER) {
            do
                return_value = subrange_selection(ENUMED);
            while (return_value == -1);
        } else {
            auto&& CHUNKED = ENUMED | std::views::chunk(MAX_IDX);
            const size_t N_PAGES = CHUNKED.size();
            auto&& PAGE_CLAMP = std::bind(std::clamp<int16_t>, std::placeholders::_1, 0, N_PAGES - 1);
            int16_t current_page = 0, pressed_key = 0;
            do {
                clear_border();
                if (pressed_key == Keys::LEFT_ARR)
                    current_page = PAGE_CLAMP(--current_page);
                else if (pressed_key == Keys::RIGHT_ARR)
                    current_page = PAGE_CLAMP(++current_page);

                auto&& current_chunk = CHUNKED[current_page];
                for (auto&& [idx, elem] : current_chunk)
                    writeln(std::format(ENUMED_ELEM_FMT, idx, elem));
                if (pressed_key == Keys::ENTER) {
                    if ((return_value = subrange_selection(current_chunk)) != -1)
                        break;
                    writeln("Нажмите ESCAPE чтобы снова выбрать нужную страницу");
                }
                write(std::format("{} страница из {}", current_page + 1, N_PAGES));
            } while (pressed_key = _getch());
        }
        return vec[return_value];
    }

    static inline void clear_border() {
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const std::string empty_space(CON_WIDTH - BORDER_PADDING, ' ');
        for (size_t i = 1; i <= CON_HEIGHT - BORDER_PADDING; i++)
            Console::putStr(empty_space, {1, int16_t(i)});
        Console::setCursorPos({1, 1});
    }
};

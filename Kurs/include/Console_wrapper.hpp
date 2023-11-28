#pragma once
#define NOMINMAX
#include <conio.h>

#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include "Console.hpp"
#include "Log.hpp"
#include "thirdparty/json.hpp"

constexpr inline const char* ENUMED_ELEM_FMT = "{0: >2}) {1}";

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

[[nodiscard]] inline std::vector<std::string> split_lines(std::string_view str) {
    return str | std::views::split('\n') | std::ranges::to<std::vector<std::string>>();
}

// for same size in different languages
[[nodiscard]] constexpr inline size_t my_strlen(std::string_view str) {
    return std::ranges::count_if(str, [](char byte) { return ((byte & 0x80) == 0 || (byte & 0xC0) == 0xC0); });
}

[[nodiscard]] constexpr inline std::string cut_str(std::string_view str, size_t count) {
    return std::string{str.substr(0, count - 3)} + "...";
}

class Console_wrapper {
   private:
    static inline char vert_symb = '|', hor_symb = '-';
    static constexpr uint8_t BORDER_PADDING = 2;

   public:
    static inline void new_line() {
        const int16_t CON_HEIGHT = Console::getSizeByChars().height;
        const int16_t CURRENT_Y = Console::getCursorPosition().height;
        const int16_t Y_NEXT = std::clamp<int16_t>(CURRENT_Y + 1, CURRENT_Y, CON_HEIGHT - BORDER_PADDING);
        Console::setCursorPos({1, Y_NEXT});
    }

    static void setup_border(char vert, char hor) {
        vert_symb = vert;
        hor_symb = hor;
    }

    static void vec_write(const std::vector<std::string>& DATA, bool enumerate = true) {
        if (DATA.empty()) {
            Logger::Error("Вектор пуст!");
            return;
        }
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getCursorPosition();
        const int32_t ACTUAL_HEIGHT = CON_HEIGHT - BORDER_PADDING;

        const int32_t VERT_SIZE_DIFF = ACTUAL_HEIGHT - DATA.size();
        const bool VEC_IS_BIGGER = VERT_SIZE_DIFF < 0;
        const size_t MAX_IDX = VEC_IS_BIGGER ? ACTUAL_HEIGHT - 1 : DATA.size();

        const auto& ENUMED = DATA | std::views::enumerate;
        auto&& print_subrange = [enumerate](auto&& ENUMED_subrange) -> void {
            for (auto&& [idx, data] : ENUMED_subrange)
                writeln(enumerate ? std::format(ENUMED_ELEM_FMT, idx + 1, data) : data);
        };

        if (!VEC_IS_BIGGER)
            print_subrange(ENUMED);
        else {
            const auto& CHUNKED = ENUMED | std::views::chunk(MAX_IDX);
            const size_t N_PAGES = CHUNKED.size();
            auto&& page_clamp = std::bind(std::clamp<int16_t>, std::placeholders::_1, 0, N_PAGES - 1);
            int16_t current_page = 0, pressed_key = 0;
            do {
                clear_border();
                if (pressed_key == Keys::LEFT_ARR)
                    current_page = page_clamp(--current_page);
                else if (pressed_key == Keys::RIGHT_ARR)
                    current_page = page_clamp(++current_page);
                print_subrange(CHUNKED[current_page]);
                write(std::format("{} страница из {}", current_page + 1, N_PAGES));
            } while ((pressed_key = _getch()) != Keys::ENTER);
        }
    }

    static void draw_frame(std::string title = "") {
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();

        const size_t TITLE_LEN = my_strlen(title);
        const bool ENABLE_TITLE = TITLE_LEN > 0 && TITLE_LEN < CON_WIDTH;
        if (ENABLE_TITLE) {
            const std::string N_SYMB((CON_WIDTH / 2) - (TITLE_LEN / 2), hor_symb);
            title = N_SYMB + title + N_SYMB;
        }

        const std::string HOR_BORDER(CON_WIDTH, hor_symb);
        const std::string VERT_BORDER = vert_symb + std::string(CON_WIDTH - BORDER_PADDING, ' ') + vert_symb;
        const int16_t MAX_Y_POS = int16_t(CON_HEIGHT - 1);  // -1 for 0-indexed count

        Console::putStr(ENABLE_TITLE ? title : HOR_BORDER, {0, 0});  // top line
        Console::putStr(HOR_BORDER, {0, MAX_Y_POS});                 // bottom line
        for (int16_t i = 1; i < MAX_Y_POS; i++)                      // vertical borders
            Console::putStr(VERT_BORDER, {0, i});                    //
        Console::setCursorPos({1, 1});
    }

    static void write(std::string_view str) {
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getCursorPosition();
        const int32_t ACTUAL_WIDTH = CON_WIDTH - BORDER_PADDING;
        const size_t STR_LENGTH = my_strlen(str);
        const ptrdiff_t FITTED_PART = ACTUAL_WIDTH - (CURSOR_X + STR_LENGTH);
        std::cout << (FITTED_PART >= 0 ? str : cut_str(str, FITTED_PART + STR_LENGTH));
    }

    static inline void writeln(std::string_view msg) {
        write(msg);
        new_line();
    }

    template <typename T>
    [[nodiscard]] constexpr static T get_inline_input(bool password = false, char symb = '*') {
        T buf{};
        if constexpr (std::is_same_v<T, std::string>) {
            do {
                const char KEY = _getch();
                if (KEY == Keys::ENTER) {
                    if (buf.empty())
                        continue;
                    new_line();
                    break;
                }
                if (KEY == Keys::BACKSPACE) {
                    if (!buf.empty()) {
                        buf.pop_back();
                        write("\b \b");
                    }
                    continue;
                }
                buf.push_back(KEY);
                write(std::string(1, password ? symb : KEY));
            } while (true);
        } else
            (std::cin >> buf).get();
        return buf;
    }

    template <typename T>
    [[nodiscard]] constexpr static T get_input(bool password = false, char symb = '*') {
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getCursorPosition();

        Console::putStr(std::string(CON_WIDTH - BORDER_PADDING, hor_symb), {1, int16_t(CON_HEIGHT - 3)});
        Console::putStr(std::string(CON_WIDTH - BORDER_PADDING, ' '), {1, int16_t(CON_HEIGHT - BORDER_PADDING)});
        Console::putStr("Ввод: ", {1, int16_t(CON_HEIGHT - BORDER_PADDING)});
        auto&& buf = get_inline_input<T>(password, symb);
        std::cin.clear();
        Console::setCursorPos({CURSOR_X, CURSOR_Y});
        return buf;
    }

    template <typename Ret_Type>
    [[nodiscard]] static Ret_Type vec_selection(const std::vector<std::string>& DATA, bool enumerate = true) {
        if (DATA.empty()) {
            Logger::Error("Вектор пуст!");
            return {};
        }
        int32_t return_value = -1;
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getCursorPosition();
        const int32_t ACTUAL_HEIGHT = CON_HEIGHT - BORDER_PADDING;

        const int32_t VERT_SIZE_DIFF = ACTUAL_HEIGHT - DATA.size();
        const bool VEC_IS_BIGGER = VERT_SIZE_DIFF < 0;
        const size_t MAX_IDX = VEC_IS_BIGGER ? ACTUAL_HEIGHT - 1 : DATA.size();
        const auto& ENUMED = DATA | std::views::enumerate;

        const auto subrange_selection = [enumerate](const auto& subrange) -> int32_t {
            clear_border();
            const int32_t IDX_MIN = std::get<0>(subrange.front());
            const int32_t IDX_MAX = std::get<0>(subrange.back());
            auto&& in_page_clamp = std::bind(std::clamp<int32_t>, std::placeholders::_1, IDX_MIN, IDX_MAX);
            int32_t scoped_idx = IDX_MIN, pressed_key = 0;
            do {
                if (pressed_key == Keys::DOWN_ARR)
                    scoped_idx = in_page_clamp(++scoped_idx);
                else if (pressed_key == Keys::UP_ARR)
                    scoped_idx = in_page_clamp(--scoped_idx);
                else if (pressed_key == Keys::ENTER)
                    break;
                else if (pressed_key == Keys::ESCAPE)
                    return -1;

                clear_border();
                for (auto&& [idx, data] : subrange) {
                    const std::string STR = ' ' + (enumerate ? std::format(ENUMED_ELEM_FMT, idx + 1, data) : data);
                    scoped_idx == idx ? writeln('>' + STR) : writeln(STR);
                }
            } while (pressed_key = _getch());
            return scoped_idx;
        };

        if (!VEC_IS_BIGGER) {
            do
                return_value = subrange_selection(ENUMED);
            while (return_value == -1);
        } else {
            const auto& CHUNKED = ENUMED | std::views::chunk(MAX_IDX);
            const size_t N_PAGES = CHUNKED.size();
            auto&& page_clamp = std::bind(std::clamp<int16_t>, std::placeholders::_1, 0, N_PAGES - 1);
            int16_t current_page = 0, pressed_key = 0;
            do {
                clear_border();
                if (pressed_key == Keys::LEFT_ARR)
                    current_page = page_clamp(--current_page);
                else if (pressed_key == Keys::RIGHT_ARR)
                    current_page = page_clamp(++current_page);

                const auto& CURRENT_CHUNK = CHUNKED[current_page];
                for (auto&& [idx, data] : CURRENT_CHUNK)
                    writeln(enumerate ? std::format(ENUMED_ELEM_FMT, idx + 1, data) : data);
                if (pressed_key == Keys::ENTER) {
                    if ((return_value = subrange_selection(CURRENT_CHUNK)) != -1)
                        break;
                    writeln("Нажмите ESCAPE чтобы снова выбрать нужную страницу");
                }
                write(std::format("{} страница из {}", current_page + 1, N_PAGES));
            } while (pressed_key = _getch());
        }
        if constexpr (std::is_integral_v<Ret_Type>)
            return return_value;
        else if constexpr (std::is_same_v<Ret_Type, std::string>)
            return DATA[return_value];
    }

    static inline void clear_border() {
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const std::string EMPTY_SPACE(CON_WIDTH - BORDER_PADDING, ' ');
        for (int16_t i = 1; i <= CON_HEIGHT - BORDER_PADDING; i++)
            Console::putStr(EMPTY_SPACE, {1, i});
        Console::setCursorPos({1, 1});
    }

    static void book_table(const nlohmann::json& js_obj, bool enumerate = true) {
        if (js_obj.empty() || js_obj.is_null()) {
            Logger::Error("Пусто!");
            return;
        }

        constexpr auto remove_quotes = [](auto&& str) {
            return (str.front() == '"' ? str.substr(1, str.length() - 2) : str);
        };
        constexpr auto format_str = [](const std::string& str, uint16_t width) {
            if (width <= str.size()) return str;
            const auto fmt_str = std::format("{{: ^{}}}", width);
            return std::vformat(fmt_str, std::make_format_args(str));
        };
        const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
        const auto [CURSOR_X, CURSOR_Y] = Console::getCursorPosition();

        const size_t COL_N = js_obj.front().size() + 1;  // 1 for title of json_obj
        std::map<std::string, size_t> s_sz{};
        for (auto&& [key, value] : js_obj.items()) {  // key = title, value = subjson
            s_sz["Title"] = std::max(s_sz["Title"], remove_quotes(key).length());
            for (auto&& [key2, value2] : value.items()) {
                const auto V_AS_STR = remove_quotes(nlohmann::to_string(value2));
                s_sz[key2] = std::max(s_sz[key2], V_AS_STR.length());
            }
        }
        const auto MAP_VALS = s_sz | std::views::values | std::views::common;
        const size_t SUM_OF_LEN = std::accumulate(MAP_VALS.begin(), MAP_VALS.end(), 0);

        const int32_t ACTUAL_HEIGHT = CON_HEIGHT - BORDER_PADDING - 1; // 1 for table header
        const int32_t ACTUAL_WIDTH = CON_WIDTH - BORDER_PADDING;

        const int32_t free_hor_space = ACTUAL_WIDTH - SUM_OF_LEN;

        if (free_hor_space >= 0) {
            const auto& last_key = s_sz.rbegin()->first;
            int16_t EACH_FREE_SPACE = free_hor_space / COL_N;
            int16_t last_free = EACH_FREE_SPACE - 1;
            
            for (auto&& [k, v] : s_sz) {
                std::cout << format_str(k, v + last_free);
                if (last_free > 0  && last_key != k)
                    std::cout << '|';
            }
            std::cout << '\n';
        }
    }
};

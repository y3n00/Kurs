#pragma once
#define NOMINMAX
#include <conio.h>

#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

#include "Console.hpp"
#include "Log.hpp"
#include "Utils.hpp"
#include "thirdparty/json.hpp"

constexpr inline const char* enumed_range_ELEM_FMT = "{0: >2}) {1}";

enum Sort_Method : uint8_t {
    less,
    greater
};

enum Keys : uint16_t {
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

struct Console_wrapper {
    static inline char vert_symb = '|', hor_symb = '-';
    static constexpr uint8_t BORDER_PADDING = 2;
    static inline int16_t CON_WIDTH{}, CON_HEIGHT{}, CURSOR_X{}, CURSOR_Y{};

    static inline void update() {
        auto&& [W, H] = Console::getSizeByChars();
        auto&& [X, Y] = Console::getCursorPosition();
        CON_WIDTH = W, CON_HEIGHT = H, CURSOR_X = X, CURSOR_Y = Y;
    }

    static inline void new_line() {
        new_cursor_pos({1, std::clamp<int16_t>(CURSOR_Y + 1, CURSOR_Y, CON_HEIGHT - BORDER_PADDING)});
    }

    static inline void setup_border(char vert, char hor) {
        vert_symb = vert;
        hor_symb = hor;
    }

    static inline void new_cursor_pos(const Console::SZ<int16_t>& new_pos) {
        CURSOR_X = new_pos.width, CURSOR_Y = new_pos.height;
        Console::setCursorPos(new_pos);
    }

    static void vec_write(const std::vector<std::string>& DATA, bool enumerate = true, std::string_view header = "") {
        if (DATA.empty()) {
            Logger::Error("Вектор пуст!");
            return;
        }
        draw_frame();
        const bool ACTIVE_HEADER = !header.empty();
        const int32_t REAL_HEIGHT = CON_HEIGHT - BORDER_PADDING - int(ACTIVE_HEADER);
        const int32_t CHUNKED_SZ = REAL_HEIGHT - 1;

        auto&& enumed_range = DATA | std::views::enumerate;
        auto&& print_subrange = [&](auto&& subrange) {
            if (ACTIVE_HEADER) writeln(header);
            for (auto&& [idx, data] : subrange)
                writeln(enumerate ? std::format(enumed_range_ELEM_FMT, idx + 1, data) : data);
        };

        if (int32_t(REAL_HEIGHT - DATA.size()) >= 0)
            print_subrange(enumed_range);
        else {
            auto&& chunked = enumed_range | std::views::chunk(CHUNKED_SZ);
            const size_t N_PAGES = chunked.size();
            auto&& page_clamp = std::bind(std::clamp<int16_t>, std::placeholders::_1, 0, N_PAGES - 1);
            int16_t current_page = 0, pressed_key = 0;
            do {
                draw_frame();
                if (pressed_key == Keys::LEFT_ARR)
                    current_page = page_clamp(--current_page);
                else if (pressed_key == Keys::RIGHT_ARR)
                    current_page = page_clamp(++current_page);
                print_subrange(chunked[current_page]);
                write(std::format("{} страница из {}", current_page + 1, N_PAGES));
            } while ((pressed_key = _getch()) != Keys::ENTER);
        }
    }

    static void draw_frame(std::string title = "") {
        update();
        const size_t TITLE_LEN = my_strlen(title);
        const bool ENABLE_TITLE = TITLE_LEN > 0 && TITLE_LEN < CON_WIDTH;
        if (ENABLE_TITLE) {
            const std::string N_SYMB((CON_WIDTH / 2) - (TITLE_LEN / 2), hor_symb);
            title = N_SYMB + title + N_SYMB;
        }

        const std::string HOR_BORDER(CON_WIDTH, hor_symb);
        const std::string VERT_BORDER = vert_symb + std::string(CON_WIDTH - BORDER_PADDING, ' ') + vert_symb;
        const int16_t MAX_Y_POS = CON_HEIGHT - 1;  // -1 for 0-indexed count

        Console::putStr(ENABLE_TITLE ? title : HOR_BORDER, {0, 0});  // top line
        Console::putStr(HOR_BORDER, {0, MAX_Y_POS});                 // bottom line
        for (int16_t i = 1; i < MAX_Y_POS; i++)                      // vertical borders
            Console::putStr(VERT_BORDER, {0, i});                    //
        new_cursor_pos({1, 1});
    }

    static void write(std::string_view str) {
        const int32_t ACTUAL_WIDTH = CON_WIDTH - BORDER_PADDING;
        const size_t STR_LENGTH = my_strlen(str);
        const ptrdiff_t FITTED_PART = ACTUAL_WIDTH - (CURSOR_X + STR_LENGTH);
        std::cout << (FITTED_PART >= 0 ? str : cut_str(str, STR_LENGTH + FITTED_PART));  // FITTED_PART is negative
    }

    static inline void writeln(std::string_view msg) {
        write(msg);
        new_line();
    }

    template <typename T>
    [[nodiscard]] constexpr static T get_inline_input(bool password = false, char symb = '*') {
        update();
        T buf{};
        if constexpr (std::is_same_v<T, std::string>) {
            do {
                const char KEY = _getch();
                if (KEY == Keys::ENTER) {
                    if (buf.empty())
                        continue;                    
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
        new_line();
        return buf;
    }

    template <typename T>
    [[nodiscard]] constexpr static T get_input(bool password = false, char symb = '*') {
        Console::putStr(std::string(CON_WIDTH - BORDER_PADDING, hor_symb), {1, int16_t(CON_HEIGHT - 3)});
        Console::putStr(std::string(CON_WIDTH - BORDER_PADDING, ' '), {1, int16_t(CON_HEIGHT - BORDER_PADDING)});
        Console::putStr("Ввод: ", {1, int16_t(CON_HEIGHT - BORDER_PADDING)});
        auto&& buf = get_inline_input<T>(password, symb);
        new_cursor_pos({CURSOR_X, CURSOR_Y});
        return buf;
    }

    template <typename Ret_Type>
    [[nodiscard]] static Ret_Type vec_selection(const std::vector<std::string>& DATA, bool enumerate = true, std::string_view header = "") {
        if (DATA.empty()) {
            Logger::Error("Вектор пуст!");
            return {};
        }
        draw_frame();
        int32_t selected_idx = -1;
        const bool ACTIVE_HEADER = !header.empty();
        const int32_t REAL_HEIGHT = CON_HEIGHT - BORDER_PADDING - int(ACTIVE_HEADER);
        const int32_t CHUNKED_SZ = REAL_HEIGHT - 1;
        auto&& print_header = [&] { if (ACTIVE_HEADER) writeln(header); };

        auto&& enumed_range = DATA | std::views::enumerate;
        auto&& subrange_selection = [&](const auto& subrange) -> int32_t {
            auto&& keys = subrange | std::views::keys | std::views::common;
            auto&& in_page_clamp = std::bind(std::clamp<int32_t>, std::placeholders::_1, keys.front(), keys.back());
            int32_t scoped_idx = in_page_clamp(0), pressed_key = 0;
            do {
                if (pressed_key == Keys::DOWN_ARR)
                    scoped_idx = in_page_clamp(++scoped_idx);
                else if (pressed_key == Keys::UP_ARR)
                    scoped_idx = in_page_clamp(--scoped_idx);
                else if (pressed_key == Keys::ENTER)
                    break;
                else if (pressed_key == Keys::ESCAPE)
                    return -1;

                draw_frame();
                print_header();
                for (auto&& [idx, data] : subrange) {
                    auto formatted_str = enumerate ? std::format(enumed_range_ELEM_FMT, idx + 1, data) : data;
                    writeln((scoped_idx == idx ? std::format("> {}", formatted_str) : formatted_str));
                }
                writeln("Нажмите ENTER чтобы подтвердить выбор");
            } while (pressed_key = _getch());
            return scoped_idx;
        };

        if (int32_t(REAL_HEIGHT - DATA.size()) >= 0)
            while ((selected_idx = subrange_selection(enumed_range)) == -1)
                ;
        else {
            const auto& chunked = enumed_range | std::views::chunk(CHUNKED_SZ);
            const size_t N_PAGES = chunked.size();
            auto&& page_clamp = std::bind(std::clamp<int16_t>, std::placeholders::_1, 0, N_PAGES - 1);
            int16_t current_page = 0, pressed_key = 0;
            do {
                draw_frame();
                if (pressed_key == Keys::LEFT_ARR)
                    current_page = page_clamp(--current_page);
                else if (pressed_key == Keys::RIGHT_ARR)
                    current_page = page_clamp(++current_page);

                const auto& CURRENT_CHUNK = chunked[current_page];
                print_header();
                for (auto&& [idx, data] : CURRENT_CHUNK)
                    writeln(enumerate ? std::format(enumed_range_ELEM_FMT, idx + 1, data) : data);
                if (pressed_key == Keys::ENTER) {
                    if ((selected_idx = subrange_selection(CURRENT_CHUNK)) != -1)
                        break;
                    writeln("Нажмите ESCAPE чтобы снова выбрать нужную страницу");
                }
                write(std::format("{} страница из {}, нажите Enter чтобы начать выбор строки", current_page + 1, N_PAGES));
            } while (pressed_key = _getch());
        }
        if constexpr (std::is_integral_v<Ret_Type>)
            return selected_idx;
        else if constexpr (std::is_same_v<Ret_Type, std::string>)
            return DATA[selected_idx];
    }

    class Table {
       private:
        constexpr static inline int16_t EACH_PADDING = 1, MAX_STRLEN = 25;
        constexpr static inline auto&& restrict_len = std::bind(std::clamp<size_t>, std::placeholders::_1, 0, MAX_STRLEN);

       private:
        std::string table_header, last_key;
        std::vector<std::string> table_rows{};
        std::vector<nlohmann::json> json_objects{};
        std::map<std::string, size_t> max_cols_widths{};

        Table* calc_col_width() {
            for (auto&& j : json_objects) {
                for (auto&& [key, value] : j.items()) {
                    auto& current = max_cols_widths[key];
                    current = restrict_len(std::max({current, value_view(value).length(), key.length()}));
                }
            }
            if (!max_cols_widths.empty())
                last_key = max_cols_widths.rbegin()->first;
            return this;
        }

        Table* generate_rows() {
            table_rows.clear();
            for (auto&& j : json_objects) {
                std::string row_buf;
                for (auto&& [key, col_width] : max_cols_widths) {
                    const auto prepared = cut_str(value_view(j[key]), MAX_STRLEN);
                    row_buf += place_by_width(prepared, col_width + EACH_PADDING);
                    if (key != last_key)
                        row_buf += '|';
                }
                table_rows.push_back(std::move(row_buf));
            }
            return this;
        }

        Table* generate_header() {
            calc_col_width();
            std::string header_buf;
            for (auto&& [key, col_width] : max_cols_widths) {
                header_buf += place_by_width(key, col_width + EACH_PADDING);
                if (key != last_key)
                    header_buf += '|';
            }
            table_header = std::format("\033[4m{}\033[0m", header_buf);  // underlined
            return this;
        }

       public:
        static auto create_table(const nlohmann::json& js_obj) {
            std::unique_ptr<Table> table_ptr = nullptr;
            if (js_obj.empty() || js_obj.is_null()) {
                Logger::Error("Пусто!");
                return table_ptr;
            }
            table_ptr = std::make_unique<Table>();
            table_ptr->json_objects = represent_json(js_obj);
            table_ptr->table_rows.reserve(table_ptr->json_objects.size());  // 1 object is 1 row
            return table_ptr;
        }

        void view() {
            generate_header()->generate_rows();
            const int16_t MAX_W = my_strlen(table_header) + BORDER_PADDING + 1;
            if(MAX_W > 50)
                Console::setSizeByChars({MAX_W, CON_HEIGHT});
            vec_write(table_rows, false, table_header);
        }

        template <typename Ty>
        [[nodiscard]] Ty pick() {
            generate_header()->generate_rows();
            const int16_t MAX_W = my_strlen(table_header) + BORDER_PADDING + 1;
            Console::setSizeByChars({MAX_W, CON_HEIGHT});
            auto&& selected_idx = vec_selection<int32_t>(table_rows, false, table_header);
            return Ty(json_objects[selected_idx]["Title"].get<std::string>());
        }

        Table* include_only(auto&& func) {
            auto&& temp = json_objects |
                          std::views::filter(func) |
                          std::views::transform([](auto&& j) { return j; }) |
                          std::ranges::to<decltype(json_objects)>();
            json_objects.swap(temp);
            return this;
        }

        Table* sort(std::string_view sort_key, Sort_Method sm = less) {
            auto&& f = [less = sm == Sort_Method::less, &sort_key](auto&& l, auto&& r) -> bool {
                return less ? l[sort_key] < r[sort_key] : l[sort_key] > r[sort_key];
            };
            std::ranges::sort(json_objects, f);
            return this;
        }

        [[nodiscard]] const auto& get_header() const { return table_header; }
        [[nodiscard]] const size_t get_sz() const { return json_objects.size(); }
    };
};
#define NOMINMAX
#include <conio.h>

#include <algorithm>
#include <iostream>

#include "Console.hpp"
#include "Log.hpp"

namespace Console_wrapper {
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

inline void put_str(std::string_view str, const Console::SZ<int16_t>& place) {
    Console::setCursorPos(place);
    std::cout << str;
}

inline void draw_frame(char vert, char hor) {
    static constexpr Console::SZ<int16_t> DEFAULT_PLACEHOLDER{1, 1};
    const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();
    const auto hor_border = std::string(CON_WIDTH, hor);
    const auto vert_border = vert + std::string(CON_WIDTH - 2, ' ') + vert;

    put_str(hor_border, {0, 0});                        // top line
    put_str(hor_border, {0, int16_t(CON_HEIGHT - 1)});  // bottom line

    for (int16_t i = 1; i < CON_HEIGHT - 1; i++)  // verical borders, 1 for bottom border
        put_str(vert_border, {0, i});             //
    Console::setCursorPos(DEFAULT_PLACEHOLDER);
}

inline void write(std::string_view msg) {
    const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();               // current console size
    const auto [CURSOR_X, CURSOR_Y] = Console::getWindowInfo().dwCursorPosition;  // current cursor position

    const uint8_t BORDER_PADDING = 2;
    const int32_t ACTUAL_CON_WIDTH = CON_WIDTH - BORDER_PADDING;
    const int32_t EXTRA_SPACE = (ACTUAL_CON_WIDTH - (CURSOR_X + msg.length()));
    const bool MSG_FITS = EXTRA_SPACE >= 0;

    std::string result_str;
    if (!MSG_FITS) {
        const size_t FITTED_SIZE = msg.length() + EXTRA_SPACE;             // EXTRA_SPACE is negative
        result_str = std::string{msg.substr(0, FITTED_SIZE - 3)} + "...";  // 3 for "..."
    }

    put_str(MSG_FITS ? msg : result_str, {CURSOR_X, CURSOR_Y});
}

inline void write_line(std::string_view msg) {
    write(msg);  // write in current cursor pos

    const uint8_t BORDER_PADDING = 2;
    const auto [CON_WIDTH, CON_HEIGHT] = Console::getSizeByChars();               // current console size
    const auto [CURSOR_X, CURSOR_Y] = Console::getWindowInfo().dwCursorPosition;  // current cursor position

    const auto clamped = std::clamp<int16_t>(CURSOR_Y + 1, CURSOR_Y, CON_HEIGHT - BORDER_PADDING);
    Console::setCursorPos({1, clamped});  // move cursor at the begining of the next line
}
}  // namespace Console_wrapper

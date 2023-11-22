#include <conio.h>

#include <memory>
#include <string_view>
constexpr std::string_view accs_fname = "accs.json";
constexpr std::string_view books_fname = "books.json";

#include "../include/Book.hpp"
#include "../include/Console_wrapper.hpp"
#include "../include/Fsystem.hpp"
#include "../include/Library.hpp"
#include "../include/Log.hpp"
#include "../include/User.hpp"

BOOL WINAPI on_exit_callback(DWORD reason) {
    if (reason == CTRL_CLOSE_EVENT) {
        FileSystem::save(accs_fname, User::get_all_accs());
        FileSystem::save(books_fname, Book::get_all_books());
        return true;
    }
}

int main() {
    system("cls");              // clear screen
    SetConsoleCP(65001);        // encoding for ru lang
    SetConsoleOutputCP(65001);  //

    const std::string window_title = "Library App";  // titlebar buffer
    Console::setFont(24, L"IBM Plex Mono Light");    // font setup
    Console::configure(window_title, {800, 600});    // setup console
    Logger::new_line_enabled = false;

    //////////////////////////////////////////////////////////////////////////////

    Book::load_books(books_fname);
    User::load_accounts(accs_fname);

    const std::unique_ptr<User>& USER = authorize();
    Console::setTitle(window_title + " | " + USER->get_login());
    SetConsoleCtrlHandler(on_exit_callback, true);  // save data only after successful login/reg

    const std::unique_ptr<ILibrary> LIBRARY = USER->is_admin() ? std::make_unique<Admin_lib>()
                                                               : std::make_unique<User_lib>();

    const int16_t MIN_IDX = 0, MAX_IDX = LIBRARY->get_menu_size() - 1;
    auto&& menu_choice_clamp = std::bind(std::clamp<int16_t>, std::placeholders::_1, MIN_IDX, MAX_IDX);
    do {
        Console_wrapper::draw_frame(" Меню ");
        const int16_t SELECTED = Console_wrapper::vec_selection<int16_t>(LIBRARY->get_menu());
        Console_wrapper::clear_border();
        LIBRARY->do_at(SELECTED);
        Console_wrapper::new_line();
        Logger::Warning("Нажмите любую клавишу чтобы продолжить");
    } while (_getch() != Keys::ESCAPE);
}
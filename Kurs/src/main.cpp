#include <conio.h>

#include <memory>
#include <string_view>
constexpr std::string_view users_file = "users.json";
constexpr std::string_view books_file = "generated_books.json";

#include "../include/Book.hpp"
#include "../include/Console_wrapper.hpp"
#include "../include/Fsystem.hpp"
#include "../include/Library.hpp"
#include "../include/Log.hpp"
#include "../include/User.hpp"

//#define GENERATE
#ifdef GENERATE
#include "../include/Random.hpp"
#include "../include/Utils.hpp"
int main() {
    static Random r;
    constexpr size_t N = 200;
    for (size_t i = 0; i < N; i++) {
        auto&& title = r.generate_string(5, 30);
        auto&& in_lib = (bool)(r.get(0, 1));
        auto&& year = r.get(1900, 2023);
        auto&& pages = r.get(10, 500);
        auto&& author = r.generate_string(10, 35);
        auto&& publisher = r.generate_string(4, 15);
        Book(title, in_lib, year, pages, author, publisher);
    }
    FileSystem::save(books_file, Book::get_json());
}
#else

BOOL WINAPI on_exit_callback(DWORD reason) {
    if (reason == CTRL_CLOSE_EVENT) {
        FileSystem::save(users_file, User::get_json());
        FileSystem::save(books_file, Book::get_json());
        return true;
    }
}

int main() {
    system("cls");              // clear screen
    SetConsoleCP(65001);        // encoding for ru lang
    SetConsoleOutputCP(65001);  //

    const std::string window_title = "Library App";  // titlebar buffer
    Console::setFont(22, L"Consolas");               // font setup
    Console::configure(window_title, {900, 600});    // setup console
    Logger::new_line_enabled = false;

    //////////////////////////////////////////////////////////////////////////////

    Book::load_books(books_file);
    User::load_accounts(users_file);

    const std::unique_ptr<User>& USER = authorize();
    Console::setTitle(window_title + " | " + USER->get_login());
    SetConsoleCtrlHandler(on_exit_callback, true);  // save data only after successful login/reg

    const std::unique_ptr<ILibrary> LIBRARY = USER->is_admin() ? std::make_unique<Admin_lib>()
                                                               : std::make_unique<User_lib>();
    static const auto& menu = LIBRARY->get_menu();
    auto&& menu_choice_clamp = std::bind(std::clamp<int16_t>, std::placeholders::_1, 0, menu.size() - 1);
    do {
        Console_wrapper::draw_frame(" Меню ");
        const int16_t SELECTED = Console_wrapper::vec_selection<int16_t>(menu);
        Console_wrapper::draw_frame();
        LIBRARY->do_at(SELECTED);
        Console_wrapper::new_line();
        Logger::Warning("Нажмите любую клавишу чтобы продолжить");
    } while (_getch() != Keys::ESCAPE);
}
#endif
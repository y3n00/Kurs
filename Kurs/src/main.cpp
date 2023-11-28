#include <conio.h>

#include <memory>
#include <string_view>
constexpr std::string_view accs_fname = "accs.json";
constexpr std::string_view books_fname = "generated_books.json";

#include "../include/Book.hpp"
#include "../include/Console_wrapper.hpp"
#include "../include/Fsystem.hpp"
#include "../include/Library.hpp"
#include "../include/Log.hpp"
#include "../include/User.hpp"

#define GENERATE
#ifdef GENERATE
#include "../include/Random.hpp"
int main() {
    Console::configure("TEST", { 1280, 720 });    // setup console
    const auto [CON_WIDTH2, CON_HEIGHT2] = Console::getSizeByChars();
    const auto [CURSOR_X2, CURSOR_Y2] = Console::getCursorPosition();

    static Random r;
    constexpr size_t N = 10;
    for (size_t i = 0; i < N; i++) {
        auto&& title = r.generate_string(5, 15);
        auto&& in_lib = (bool)(r.get(0, 1));
        auto&& year = r.get(1900, 2023);
        auto&& pages = r.get(10, 500);
        auto&& author = r.generate_string(10, 20);
        auto&& publisher = r.generate_string(5, 15);
        Book(title, in_lib, year, pages, author, publisher);
    }
    Console_wrapper::book_table(Book::get_books_json());
}
#else

BOOL WINAPI on_exit_callback(DWORD reason) {
    if (reason == CTRL_CLOSE_EVENT) {
        FileSystem::save(accs_fname, User::get_all_accs());
        FileSystem::save(books_fname, Book::get_books_json());
        return true;
    }
}

int main() {
    system("cls");              // clear screen
    SetConsoleCP(65001);        // encoding for ru lang
    SetConsoleOutputCP(65001);  //

    const std::string window_title = "Library App";  // titlebar buffer
    Console::setFont(24, L"Consolas");               // font setup
    Console::configure(window_title, {800, 800});    // setup console
    Logger::new_line_enabled = false;

    //////////////////////////////////////////////////////////////////////////////

    Book::load_books(books_fname);
    User::load_accounts(accs_fname);

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
        Console_wrapper::clear_border();
        LIBRARY->do_at(SELECTED);
        Console_wrapper::new_line();
        Logger::Warning("Нажмите любую клавишу чтобы продолжить");
    } while (_getch() != Keys::ESCAPE);
}
#endif
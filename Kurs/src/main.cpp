#include <conio.h>

#include <memory>

#include "../include/Book.hpp"
#include "../include/Console_wrapper.hpp"
#include "../include/Fsystem.hpp"
#include "../include/Library.hpp"
#include "../include/Log.hpp"
#include "../include/User.hpp"

constexpr std::string_view accs_fname = "accs.json";
constexpr std::string_view books_fname = "books.json";

BOOL WINAPI on_exit_callback(DWORD reason) {
    if (reason == CTRL_CLOSE_EVENT) {
        FileSystem::save(accs_fname, User::get_all_accs());
        FileSystem::save(books_fname, Book::get_all_books());
        system("cls");

        return true;
    }
}

int main() {
    system("cls");
    SetConsoleCP(65001);        // encoding for ru lang
    SetConsoleOutputCP(65001);  //

    std::string windows_title = "Library App";      // titlebar buffer
    Console::setFont(18, L"Cascadia Mono");         // font resize
    Console::Configure(windows_title, {800, 600});  // setup console

    //////////////////////////////////////////////////////////////////////////////

    Book::load_books(books_fname);
    User::load_accounts(accs_fname);

    const auto user = authorize();
    Console::setTitle(windows_title + " | " + user.get_login());
    SetConsoleCtrlHandler(on_exit_callback, true);  // only after successful login/reg

    const auto library = [is_admin = user.is_admin()] {
        return is_admin ? std::make_unique<Library_as_admin>()
                        : std::make_unique<Library_as_user>();
    }();

    static const std::string MENU_ITEMS{library->get_menu().str()};
    static const choice_t MIN_IDX = 1, MAX_IDX = library->get_menu_size();

    choice_t choice = 0;
    do {
        Console_wrapper::draw_frame('|', '-');
        Console_wrapper::writeln(MENU_ITEMS);
        const auto INPUT = Console_wrapper::get_input<choice_t>();
        choice = std::clamp(INPUT, MIN_IDX, MAX_IDX);

        Console_wrapper::clear_screen();
        library->do_at(choice);
    } while (_getch() != Keys::ESCAPE);
}
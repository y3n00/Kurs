#include <memory>

#include "../include/Book.hpp"
#include "../include/Console_wrapper.hpp"
#include "../include/Fsystem.hpp"
#include "../include/Library.hpp"
#include "../include/Log.hpp"
#include "../include/User.hpp"
constexpr std::string_view accs_fname = "accs.json";
constexpr std::string_view books_fname = "books.json";

int main() {
    SetConsoleCP(65001);        // set new encoding for ru lang
    SetConsoleOutputCP(65001);  //

    std::string windows_title = "Library App";      // dynamic titlebar buffer
    Console::Configure(windows_title, {960, 480});  // setup console with new title and size
    Console::setFont(18);                           // console font resize

    constexpr auto exit_callback = []() -> void {
        int choice = -1;
        std::print("Сохранить данные?\n1) Да\n2)Нет\n");
        std::cin >> choice;
        if (choice == 1) {
            FileSystem::save(accs_fname, User::get_all_accs());
            FileSystem::save(books_fname, Book::get_all_books());
        }
    };
    std::at_quick_exit(exit_callback);
    std::atexit(exit_callback);

    //////////////////////////////////////////////////////////////////////////////

    Book::load_books(books_fname);
    User::load_accounts(accs_fname);

    const auto user = authorize();
    Console::setTitle(windows_title + " | " + user.get_login());

    std::unique_ptr<Library_as_user> lib;
    if (user.is_admin())
        lib = std::make_unique<Library_as_admin>();
    else
        lib = std::make_unique<Library_as_user>();

    static const size_t MIN_IDX = 1, MAX_IDX = lib->get_menu_size();
    static int choice = -1;

    do {
        Console_wrapper::draw_frame('|', '-');

        Console_wrapper::writeln(lib->get_menu().str());
        std::cin >> choice;
        choice = std::clamp<int>(choice, MIN_IDX, MAX_IDX);

        lib->do_at(choice);
    } while (choice != -1);
}
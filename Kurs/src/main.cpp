#include <memory>

#include "../include/Book.hpp"
#include "../include/Console.hpp"
#include "../include/Fsystem.hpp"
#include "../include/Library.hpp"
#include "../include/Log.hpp"
#include "../include/User.hpp"
constexpr std::string_view accounts_fname = "accs.json";
constexpr std::string_view books_fname = "books.json";

int main() {
    static const auto prev_cp_val = GetConsoleCP();               // save console state
    static const auto prev_cp_output_val = GetConsoleOutputCP();  //

    SetConsoleCP(65001);        // set new encoding
    SetConsoleOutputCP(65001);  //

    std::string windows_title = "Library App";      // dynamic windows titlebar
    Console::Configure(windows_title, {960, 480});  // set new title and size
    Console::setFont(18);                           // new font size

    constexpr auto exit_callback = []() -> void {
        FileSystem::save(accounts_fname, User::get_all_accs());
        FileSystem::save(books_fname, Book::get_all_books());
        SetConsoleCP(prev_cp_val);
        SetConsoleOutputCP(prev_cp_output_val);
    };
    std::at_quick_exit(exit_callback);
    std::atexit(exit_callback);

    //////////////////////////////////////////////////////////////////////////////

    Book::load_books(books_fname);
    User::load_accounts(accounts_fname);

    const auto user = authorize();
    Console::setTitle(windows_title + " | " + user.get_login());

    std::unique_ptr<Library_as_user> lib;
    if (user.is_admin())
        lib = std::make_unique<Library_as_admin>();
    else
        lib = std::make_unique<Library_as_user>();
    lib->main_loop();
}
#include <memory>

#include "../include/Book.hpp"
#include "../include/Console.hpp"
#include "../include/Fsystem.hpp"
#include "../include/Library.hpp"
#include "../include/Log.hpp"
#include "../include/User.hpp"
constexpr std::string_view accounts_fname = "accs.json";
constexpr std::string_view books_fname = "books.json";

/*
Вывести список книг с фамилиями авторов в алфавитном порядке, изданных после
заданного года (год вводится с клавиатуры). Вывести список книг, находящихся в
текущий момент у читателей.
*/

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    std::string windows_title = "Library App";
    Console::Configure(windows_title, {800, 600});
    Console::setFont(18, L"IMB Plex Mono");
    std::atexit([] { FileSystem::save(accounts_fname, User::get_all_accs()); });
    std::atexit([] { FileSystem::save(books_fname, Book::get_all_books()); });

    Book::load_books(books_fname);
    User::load_accounts(accounts_fname);

    const auto user = authorize();
    Console::setTitle(windows_title + " | " + user.get_login());

    std::unique_ptr<Library_as_user> lib;
    if (user.is_admin())
        lib = std::make_unique<Library_as_admin>();
    else
        lib = std::make_unique<Library_as_user>();
    system("pause");
}
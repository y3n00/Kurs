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
    D_SUCCESS("WOWWWW!");
    Console::Configure("Library App", {1280, 720});

    Book::load_books(books_fname);
    User::load_accounts(accounts_fname);

    std::atexit([] { FileSystem::save(accounts_fname, User::get_all_accs()); });
    std::atexit([] { FileSystem::save(books_fname, Book::get_all_books()); });

    std::unique_ptr<Library_as_user> lib;
    const auto user = authorize();

    if (user.is_admin())
        lib = std::make_unique< Library_as_admin>();
    else
        lib = std::make_unique< Library_as_user>();
    lib->menu();

#ifdef _DEBUG
    for (const auto& i : User::get_all_accs().items()) {
        const auto&[title, data] = i;
        std::cout << title << '\t' << data << '\n';
    }
#endif 


    system("pause");
}
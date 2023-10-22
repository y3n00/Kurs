#include <memory>

#include "../include/Book.hpp"
#include "../include/Library.hpp"
#include "../include/User.hpp"

constexpr std::string_view accounts_fname = "accs.json";
constexpr std::string_view books_fname = "books.json";

int main() {
    Book::load_books(books_fname);
    User::load_accounts(accounts_fname);

    for (auto&& obj : User::get_all_accs().items()) {
        std::cout << obj.key() << " : " << obj.value() << '\n';
    }
}
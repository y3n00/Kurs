#pragma once

#include <format>
#include <ranges>
#include <sstream>
#include <vector>

#include "Book.hpp"
#include "Console_wrapper.hpp"
#include "User.hpp"

constexpr static auto MENU_ITEM_FMT = "{: >2}) {}\n";
struct FUNCTION {
    const char* name;
    void (*func)();
    constexpr void invoke() const { func(); }
};

namespace USER_Functions {
void view_all_books() noexcept {
    const auto& all_books_json = Book::get_all_books();
    std::vector<Book> books_vec;
    books_vec.reserve(all_books_json.size());
    for (const auto& [title, data] : all_books_json.items())
        books_vec.emplace_back(title, data);

    constexpr auto get_title = [](const Book& book) { return book.get_title(); };
    const size_t s = Console_wrapper::vector_selection<Book, std::string>(books_vec, get_title);
}

void search_book() {
    const auto& all_books_json = Book::get_all_books();
    Console_wrapper::clear_border();
    Console_wrapper::writeln("Введите название книги которую желаете найти");
    const auto book_to_find = Console_wrapper::get_input<std::string>();
    if (all_books_json.contains(book_to_find)) {
        Console_wrapper::writeln("Книга найдена!");
        const auto book_data = Book(book_to_find, all_books_json.at(book_to_find)).get_data();
        Console_wrapper::writeln(book_data);
    } else {
        Console_wrapper::writeln("Такой книги нет!");
        return;
    }
}
}  // namespace USER_Functions

namespace ADMIN_Functions {
void print_all_users() {
    Console_wrapper::clear_border();
    const auto& all_users_json = User::get_all_accs();
    for (auto&& [idx, acc] : all_users_json.items() | std::views::enumerate) {
        const auto& [login, js_data] = acc;
        const User user(login, js_data);
    }
}

void edit_user() {
    const auto& all_users_json = User::get_all_accs();
    std::vector<User> users_vec;
    users_vec.reserve(all_users_json.size());
    for (const auto& [login, data] : all_users_json.items())
        users_vec.emplace_back(login, data);
    constexpr auto get_login = [](const User& user) { return user.get_login(); };
    const size_t s = Console_wrapper::vector_selection<User, std::string>(users_vec, get_login);
}
}  // namespace ADMIN_Functions

class Library_as_user {
    static inline const std::vector<FUNCTION> user_funcs{
        {"просмотреть все книги", USER_Functions::view_all_books},
        {"поиск книги", USER_Functions::search_book},
        {"сортировка", [] { Console_wrapper::writeln("user #3"); }},
    };

   public:
    [[nodiscard]] virtual std::stringstream get_menu() const {
        std::stringstream sstr;

        for (const auto& [idx, FUNCTION] : user_funcs | std::views::enumerate)
            sstr << std::format(MENU_ITEM_FMT, idx + 1, FUNCTION.name);

        return sstr;
    }

    inline virtual void do_at(uint16_t idx) const {
        user_funcs.at(idx).invoke();
    }

    [[nodiscard]] inline virtual size_t get_menu_size() const {
        return user_funcs.size();
    }

    virtual ~Library_as_user() = default;
};

class Library_as_admin : virtual public Library_as_user {
    static inline const std::vector<FUNCTION> admin_funcs{
        {"просмотреть все учетные записи", ADMIN_Functions::print_all_users},
        {"добавить учетную запись", [] { Console_wrapper::writeln("admin #2"); }},
        {"отредактировать учетную запись", [] { Console_wrapper::writeln("admin #3"); }},
        {"удалить учетную запись", [] { Console_wrapper::writeln("admin #4"); }},
    };

   public:
    [[nodiscard]] std::stringstream get_menu() const override {
        const auto menu_delta = Library_as_user::get_menu_size();
        auto sstr = Library_as_user::get_menu();

        for (const auto& [idx, FUNCTION] : admin_funcs | std::views::enumerate)
            sstr << std::format(MENU_ITEM_FMT, idx + menu_delta + 1, FUNCTION.name);
        return sstr;
    }

    inline void do_at(uint16_t idx) const override {
        const auto user_menu_size = Library_as_user::get_menu_size();
        if (idx < user_menu_size)
            Library_as_user::do_at(idx);
        else
            admin_funcs.at(idx - user_menu_size).invoke();
    }

    [[nodiscard]] inline size_t get_menu_size() const override {
        return Library_as_user::get_menu_size() + admin_funcs.size();
    }
};
/* ADMIN!
1. Управление учетными записями пользователей:
- просмотреть все учетные записи;
- добавить учетную запись;
- отредактировать учетную запись;
- удалить учетную запись.
2. *Работа с файлом данных:
- создать файл;
- открыть файл;
- удалить файл.
3. Работа с данными:
а) режим редактирования:
- просмотреть все данные;
- добавить запись;
- удалить запись (для этого необходимо ввести порядковый номер конкретной записи);
- редактировать запись (для этого необходимо ввести порядковый номер конкретной
записи).
*/

/* USER
- просмотреть все данные;
- выполнить задачу (задачи), указанную в индивидуальном задании;
- выполнить поиск данных;
- выполнить сортировку по различным полям в алфавитном порядке / в порядке убывания.
*/

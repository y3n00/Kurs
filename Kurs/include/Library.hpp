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
    const size_t s = Console_wrapper::vec_selection<Book>(books_vec, get_title);
}

void search_book() {
    const auto& all_books_json = Book::get_all_books();
    Console_wrapper::clear_border();
    Console_wrapper::writeln("Введите название книги которую желаете найти");
    const auto book_to_find = Console_wrapper::get_input<std::string>();
    if (all_books_json.contains(book_to_find)) {
        Console_wrapper::writeln("Книга найдена!");
        const auto book_data = Book(book_to_find, all_books_json.at(book_to_find)).get_data();
        Console_wrapper::write_vec(book_data);
    } else {
        Console_wrapper::writeln("Такой книги нет!");
    }
}
}  // namespace USER_Functions

namespace ADMIN_Functions {
void print_all_users() {
    Console_wrapper::draw_frame();
    for (auto&& [login, js_data] : User::get_all_accs().items())
        Console_wrapper::write_vec(User(login, js_data).get_data(), false);
}

void edit_user() {
    const auto js_to_user = [](auto&& js_item) {
        auto&& [login, data] = js_item;
        return User(login, data);
    };
    constexpr auto get_login = [](const User& user) { return user.get_login(); };

    auto users_vec = User::get_all_accs().items() |
                     std::views::transform(js_to_user) |
                     std::ranges::to<std::vector<User>>();

    User& user = users_vec[Console_wrapper::vec_selection<User>(users_vec, get_login)];
    Console_wrapper::clear_border();
    Console_wrapper::writeln(std::format("Выбранный пользователь: {}", user.get_login()));
    Console_wrapper::write_vec(user.get_data());
    const uint32_t selection = Console_wrapper::get_input<uint16_t>() - 1;
    switch (selection) {
        case 0:
            Console_wrapper::write("Введите новый логин");
            user.set_login(Console_wrapper::get_input<std::string>());
            break;
        case 1:
            Console_wrapper::write("Введите новый пароль");
            user.set_password(Console_wrapper::get_input<std::string>());
            break;
        case 2:
            Console_wrapper::write("Роль была изменена!");
            const auto NEW_ROLE = !static_cast<bool>(user.get_role());
            user.set_role(NEW_ROLE);
            break;
    }
    user.update_data();
}
}  // namespace ADMIN_Functions

struct ILibrary {
    virtual std::vector<std::string> get_menu() const = 0;
    virtual void do_at(uint16_t idx) const = 0;
    virtual size_t get_menu_size() const = 0;
    virtual ~ILibrary() = default;
};

class Library_as_user : public virtual ILibrary {
    static inline const std::vector<FUNCTION> user_funcs{
        {"просмотреть все книги", USER_Functions::view_all_books},
        {"поиск книги", USER_Functions::search_book},
        {"сортировка", [] { Console_wrapper::writeln("user #3"); }},
    };

   public:
    [[nodiscard]] std::vector<std::string> get_menu() const override {
        std::vector<std::string> strings;
        for (auto&& func : user_funcs)
            strings.emplace_back(func.name);
        return strings;
    }

    inline void do_at(uint16_t idx) const override {
        user_funcs.at(idx).invoke();
    }

    [[nodiscard]] inline size_t get_menu_size() const override {
        return user_funcs.size();
    }
};

class Library_as_admin : virtual public ILibrary, public Library_as_user {
    static inline const std::vector<FUNCTION> admin_funcs{
        {"просмотреть все учетные записи", ADMIN_Functions::print_all_users},
        {"добавить учетную запись", [] { Console_wrapper::writeln("admin #2"); }},
        {"отредактировать учетную запись", ADMIN_Functions::edit_user},
        {"удалить учетную запись", [] { Console_wrapper::writeln("admin #4"); }},
    };

   public:
    [[nodiscard]] std::vector<std::string> get_menu() const override {
        auto&& menu = std::move(Library_as_user::get_menu());
        for (auto&& func : admin_funcs)
            menu.emplace_back(func.name);
        return menu;
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

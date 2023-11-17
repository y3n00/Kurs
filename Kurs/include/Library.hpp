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
void view_all_books() {
    const nlohmann::json& all_books = Book::get_all_books();
    auto&& books_vec = all_books.items() |
                       std::views::transform([](auto&& js_item) { return js_item.key(); }) |
                       std::ranges::to<std::vector<std::string>>();
    Console_wrapper::vec_print(books_vec, false);
}

void take_book() {
    const nlohmann::json& all_books = Book::get_all_books();
    if (all_books.empty()) {
        Logger::Error("Список книг пуст!");
        return;
    }
    auto&& books_vec = all_books.items() |
                       std::views::filter([](auto&& js_item) { return js_item.value().at("In library").get<bool>(); }) |
                       std::views::transform([](auto&& js_item) { return js_item.key(); }) |
                       std::ranges::to<std::vector<std::string>>();
    const std::string selected_title = Console_wrapper::vec_selection(books_vec);
    if (selected_title.empty())
        return;
    Book book(selected_title, all_books.at(selected_title));
    User::get_current_user()->take_book(book);
    book.update_data();
}

void return_book() {
    const nlohmann::json& all_books = Book::get_all_books();
    if (all_books.empty()) {
        Logger::Error("Список книг пуст!");
        return;
    }
    auto&& user = *User::get_current_user();
    auto&& user_books = user.get_book_list();
    const std::string selected_title = Console_wrapper::vec_selection(user_books);
    if (selected_title.empty())
        return;
    Book book(selected_title, all_books.at(selected_title));
    User::get_current_user()->return_book(book);
    book.update_data();
}

void search_book() {
    const nlohmann::json& all_books = Book::get_all_books();
    if (all_books.empty()) {
        Logger::Error("Список книг пуст!");
        return;
    }
    Console_wrapper::clear_border();
    Console_wrapper::writeln("Введите название книги которую желаете найти");
    std::string book_to_find = Console_wrapper::get_input<std::string>();
    bool found = false;
    for (auto&& book : all_books.items())
        if (book.key().contains(book_to_find)) {
            found = true;
            book_to_find = book.key();
            break;
        }
    if (!found)
        Console_wrapper::writeln("Такой книги нет!");
    else {
        Console_wrapper::writeln("Книга найдена!");
        Console_wrapper::vec_print(Book(book_to_find, all_books.at(book_to_find)).get_data(), false);
    }
}
}  // namespace USER_Functions

namespace ADMIN_Functions {
void print_all_users() {
    const nlohmann::json& all_accs = User::get_all_accs();
    auto&& logins_vec = all_accs.items() |
                        std::views::transform([](auto&& js_item) { return js_item.key(); }) |
                        std::ranges::to<std::vector<std::string>>();
    const std::string selected_login = Console_wrapper::vec_selection(logins_vec);
    const nlohmann::json& js_data = all_accs.at(selected_login);
    User user(selected_login, js_data);
    Console_wrapper::clear_border();
    Console_wrapper::writeln(std::format("Выбранный пользователь: {}", selected_login));
    Console_wrapper::vec_print(user.get_data(), false);
}

void edit_user() {
    const nlohmann::json& all_accs = User::get_all_accs();
    auto&& logins_vec = all_accs.items() |
                        std::views::transform([](auto&& js_item) { return js_item.key(); }) |
                        std::ranges::to<std::vector<std::string>>();
    const std::string selected_login = Console_wrapper::vec_selection(logins_vec);
    const nlohmann::json& js_data = all_accs.at(selected_login);
    User user(selected_login, js_data);

    Console_wrapper::clear_border();
    Console_wrapper::writeln(std::format("Выбранный пользователь: {}", selected_login));
    Console_wrapper::writeln("Выберите то, что желаете изменить:");
    Console_wrapper::vec_print(user.get_data());
    switch (Console_wrapper::get_input<uint16_t>()) {
        case 1:
            Console_wrapper::write("Введите новый логин");
            user.set_login(Console_wrapper::get_input<std::string>());
            break;
        case 2:
            Console_wrapper::write("Введите новый пароль");
            user.set_password(Console_wrapper::get_input<std::string>());
            break;
        case 3:
            Console_wrapper::write("Роль была изменена!");
            user.set_role(User_role(!bool(user.get_role())));
            break;
        default:
            return;
    }
    user.update_data();
}

void delete_user_entry() {
    const nlohmann::json& all_accs = User::get_all_accs();
    auto&& logins_vec = all_accs.items() |
                        std::views::transform([](auto&& js_item) { return js_item.key(); }) |
                        std::ranges::to<std::vector<std::string>>();
    const std::string selected_login = Console_wrapper::vec_selection(logins_vec);
    User::erase(selected_login);
}
}  // namespace ADMIN_Functions

struct ILibrary {
    virtual std::vector<std::string> get_menu() const = 0;
    virtual size_t get_menu_size() const = 0;
    virtual void do_at(uint16_t idx) const = 0;
    virtual ~ILibrary() = default;
};

class Library_as_user : public virtual ILibrary {
   protected:
    static inline const std::vector<FUNCTION> user_funcs{
        {"просмотреть все книги", USER_Functions::view_all_books},
        {"поиск книги", USER_Functions::search_book},
        {"взять книгу", USER_Functions::take_book},
        {"вернуть книгу", USER_Functions::return_book},
        {"сортировка", [] { Console_wrapper::writeln("user #3"); }},
    };

   public:
    [[nodiscard]] std::vector<std::string> get_menu() const override {
        return user_funcs |
               std::views::transform([](const FUNCTION& func) { return func.name; }) |
               std::ranges::to<std::vector<std::string>>();
    }

    inline void do_at(uint16_t idx) const override {
        user_funcs.at(idx).invoke();
    }

    [[nodiscard]] inline size_t get_menu_size() const override {
        return user_funcs.size();
    }
};

class Library_as_admin : virtual public ILibrary, public Library_as_user {
   protected:
    static inline const std::vector<FUNCTION> admin_funcs{
        {"просмотреть все учетные записи", ADMIN_Functions::print_all_users},
        {"добавить учетную запись", [] { Console_wrapper::writeln("admin #2"); }},
        {"отредактировать учетную запись", ADMIN_Functions::edit_user},
        {"удалить учетную запись", ADMIN_Functions::delete_user_entry},
    };

   public:
    [[nodiscard]] std::vector<std::string> get_menu() const override {
        auto&& menu = std::move(Library_as_user::get_menu());
        std::ranges::move(admin_funcs |
                              std::views::transform([](const FUNCTION& func) { return func.name; }) |
                              std::ranges::to<std::vector<std::string>>(),
                          std::back_inserter(menu));
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
- добавить учетную запись;
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
- выполнить задачи, указанную в индивидуальном задании:
    Вывести  список  книг  с  фамилиями  авторов  в  алфавитном  порядке,  изданных  после
    заданного  года  (год  вводится  с  клавиатуры).  Вывести  список  книг,  находящихся  в
    текущий момент у читателей
- выполнить поиск данных;
- выполнить сортировку по различным полям в алфавитном порядке / в порядке убывания.
*/

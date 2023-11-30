#pragma once

#include <algorithm>
#include <format>
#include <ranges>
#include <sstream>
#include <vector>

#include "Book.hpp"
#include "Console_wrapper.hpp"
#include "User.hpp"

struct FUNCTION {
    const char* name;
    void (*func)();
    constexpr void invoke() const { func(); }
};

namespace USER_Functions {
    void my_task() {
        std::vector ALL_BOOKS = Book::get_all_books();
        if (ALL_BOOKS.empty()) {
            Logger::Error("К сожалению список книг пока пуст!");
            return;
        }
        Console_wrapper::write("Введите год: ");
        const uint16_t YEAR = Console_wrapper::get_inline_input<uint16_t>();
        auto&& BOOKS_BY_PARAM = ALL_BOOKS |
                                std::views::filter([YEAR](auto&& book) { return book.is_in_library() && book.get_year() >= YEAR; }) |
                                std::views::transform([](auto&& book) { return std::move(book); }) |
                                std::ranges::to<std::vector<Book>>();

        // std::sort(BOOKS_BY_PARAM.begin(), BOOKS_BY_PARAM.end(),
        //           [](auto&& b1, auto&& b2) -> bool {
        //               return b1.get_author() < b2.get_author();
        //           });
        // Console_wrapper::book_table(BOOKS_BY_PARAM);
    }

    void view_all_books() {
        const std::vector BOOKS = Book::get_all_books() |
                                  std::views::transform([](auto&& book) { return book.get_title(); }) |
                                  std::ranges::to<std::vector<std::string>>();
        Console_wrapper::vec_write(BOOKS, true);
    }

    void take_book() {
        const std::vector ALL_BOOKS = Book::get_all_books();
        if (ALL_BOOKS.empty()) {
            Logger::Error("К сожалению список книг пока пуст!");
            return;
        }
        const std::vector BOOKS = ALL_BOOKS |
                                  std::views::filter([](auto&& book) { return book.is_in_library(); }) |
                                  std::views::transform([](auto&& book) { return book.get_title(); }) |
                                  std::ranges::to<std::vector<std::string>>();
        Book book(Console_wrapper::vec_selection<std::string>(BOOKS, false));
        User::get_current_user()->take_book(book);
        book.update_data();
        Logger::Succsess("Книга взята!");
    }

    void return_book() {
        const std::vector ALL_BOOKS = Book::get_all_books();
        if (ALL_BOOKS.empty()) {
            Logger::Error("К сожалению список книг пока пуст!");
            return;
        }
        User& user = *User::get_current_user();
        const auto taken_books = user.get_taken_books();
        if (taken_books.empty()) {
            Logger::Error("Нет взятых книг!");
            return;
        }
        Book book(Console_wrapper::vec_selection<std::string>(taken_books));
        user.return_book(book);
        book.update_data();
        Logger::Succsess("Книга возвращена!");
    }

    void search_book() {
        const std::vector ALL_BOOKS = Book::get_all_books();
        if (ALL_BOOKS.empty()) {
            Logger::Error("К сожалению список книг пока пуст!");
            return;
        }
        Console_wrapper::writeln("Введите название книги которую желаете найти");
        const std::string TO_FIND = Console_wrapper::get_input<std::string>();
        for (auto&& book : ALL_BOOKS) {
            if (std::string title = book.get_title(); title.contains(TO_FIND)) {
                Console_wrapper::writeln("Книга найдена!");
                Console_wrapper::vec_write(Book(title).get_data(), false);
                return;
            }
        }
        Logger::Error("Такой книги нет!");
    }
}  // namespace USER_Functions

namespace ADMIN_Functions {
    void print_all_users() {
        const std::vector ALL_USERS = User::get_all_users();
        if (ALL_USERS.empty()) {
            Logger::Error("Список пользователей пуст!");
            return;
        }
        const std::vector LOGINS = ALL_USERS |
                                   std::views::transform([](auto&& user) { return user.get_login(); }) |
                                   std::ranges::to<std::vector<std::string>>();
        const User USER(Console_wrapper::vec_selection<std::string>(LOGINS));
        Console_wrapper::clear_border();
        Console_wrapper::writeln(std::format("Выбранный пользователь: {}", USER.get_login()));
        Console_wrapper::vec_write(USER.get_data(), false);
    }

    void edit_user() {
        const std::vector ALL_USERS = User::get_all_users();
        if (ALL_USERS.empty()) {
            Logger::Error("Список пользователей пуст!");
            return;
        }
        const std::vector LOGINS = ALL_USERS |
                                   std::views::transform([](auto&& user) { return user.get_login(); }) |
                                   std::ranges::to<std::vector<std::string>>();
        User user(Console_wrapper::vec_selection<std::string>(LOGINS));

        Console_wrapper::clear_border();
        Console_wrapper::writeln(std::format("Выбранный пользователь: {}", user.get_login()));
        Console_wrapper::writeln("Выберите то, что желаете изменить:");
        Console_wrapper::writeln("1) Логин");
        Console_wrapper::writeln("2) Пароль");
        Console_wrapper::writeln("3) Статус");
        switch (Console_wrapper::get_input<uint16_t>()) {
            case 1:
                Console_wrapper::writeln("Введите новый логин");
                user.set_login(Console_wrapper::get_input<std::string>());
                break;
            case 2:
                Console_wrapper::writeln("Введите новый пароль");
                user.set_password(Console_wrapper::get_input<std::string>(true));
                break;
            case 3:
                Console_wrapper::writeln("Роль была изменена!");
                user.set_role(User_role(!bool(user.get_role())));
                break;
            default:
                return;
        }
        user.update_data();
        Console_wrapper::writeln("Новые данные сохранены");
    }

    void delete_user_entry() {
        const std::vector ALL_USERS = User::get_all_users();
        if (ALL_USERS.empty()) {
            Logger::Error("Список пользователей пуст!");
            return;
        }
        const std::vector LOGINS = ALL_USERS |
                                   std::views::transform([](auto&& user) { return user.get_login(); }) |
                                   std::ranges::to<std::vector<std::string>>();
        User::erase(Console_wrapper::vec_selection<std::string>(LOGINS));
    }

    void remove_file() {
        std::vector<std::string> files;
        for (auto&& f : std::filesystem::directory_iterator(std::filesystem::current_path())) {
            if (const auto& f_path = f.path(); f_path.extension() == ".json")
                files.emplace_back(f_path.filename().string());
        }
        Console_wrapper::writeln("Выберите файл");
        const std::string TO_REMOVE = Console_wrapper::vec_selection<std::string>(files);
        Console_wrapper::writeln(std::format("Вы уверены, что хотите удалить {}?", TO_REMOVE));
        Console_wrapper::writeln("1) Да");
        Console_wrapper::writeln("2) Нет");
        if (Console_wrapper::get_input<uint16_t>() == 1) {
            std::filesystem::remove(TO_REMOVE);
            Logger::Succsess("Файл удален!");
        } else
            Logger::Warning("Действие было отменено");
    }
}  // namespace ADMIN_Functions

struct ILibrary {
    constexpr virtual std::vector<std::string> get_menu() const = 0;
    constexpr virtual size_t get_menu_size() const = 0;
    constexpr virtual void do_at(uint16_t idx) const = 0;
    constexpr virtual ~ILibrary() = default;
};

class User_lib : public virtual ILibrary {
   protected:
    static inline const std::vector<FUNCTION> user_funcs{
        {"задание по варианту", USER_Functions::my_task},
        {"просмотреть все книги", USER_Functions::view_all_books},
        {"поиск книги", USER_Functions::search_book},
        {"взять книгу", USER_Functions::take_book},
        {"вернуть книгу", USER_Functions::return_book},
        {"!сортировка", [] { Console_wrapper::writeln("user #5"); }},
    };

   public:
    [[nodiscard]] constexpr std::vector<std::string> get_menu() const override {
        return user_funcs |
               std::views::transform([](const FUNCTION& func) { return func.name; }) |
               std::ranges::to<std::vector<std::string>>();
    }

    constexpr inline void do_at(uint16_t idx) const override {
        user_funcs.at(idx).invoke();
    }

    [[nodiscard]] constexpr inline size_t get_menu_size() const override {
        return user_funcs.size();
    }
};

class Admin_lib : virtual public ILibrary, public User_lib {
   protected:
    static inline const std::vector<FUNCTION> admin_funcs{
        {"просмотреть все учетные записи", ADMIN_Functions::print_all_users},
        {"!добавить книгу", [] { Console_wrapper::writeln("admin #2"); }},
        {"!добавить учетную запись", [] { Console_wrapper::writeln("admin #3"); }},
        {"отредактировать учетную запись", ADMIN_Functions::edit_user},
        {"удалить учетную запись", ADMIN_Functions::delete_user_entry},
        {"удалить файл", ADMIN_Functions::remove_file},
    };

   public:
    [[nodiscard]] constexpr std::vector<std::string> get_menu() const override {
        auto&& menu = std::move(User_lib::get_menu());
        std::ranges::move(admin_funcs |
                              std::views::transform([](const FUNCTION& func) { return func.name; }) |
                              std::ranges::to<std::vector<std::string>>(),
                          std::back_inserter(menu));
        return menu;
    }

    constexpr inline void do_at(uint16_t idx) const override {
        const size_t USER_MENU_SIZE = User_lib::get_menu_size();
        if (idx < USER_MENU_SIZE)
            User_lib::do_at(idx);
        else
            admin_funcs.at(idx - USER_MENU_SIZE).invoke();
    }

    [[nodiscard]] constexpr inline size_t get_menu_size() const override {
        return User_lib::get_menu_size() + admin_funcs.size();
    }
};
/* ADMIN!
1. Управление учетными записями пользователей:
- добавить учетную запись;
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

#pragma once
#include <algorithm>
#include <format>
#include <ranges>
#include <sstream>
#include <vector>

#include "Book.hpp"
#include "Console_wrapper.hpp"
#include "User.hpp"
#include "Utils.hpp"

namespace USER_Functions {
    void my_task() {
        auto&& all_books_json = Book::get_json();
        if (all_books_json.empty()) {
            Logger::Error("Список книг пока пуст!");
            return;
        }
        Console_wrapper::write("Введите год: ");
        auto&& year = Console_wrapper::get_inline_input<uint16_t>();
        auto&& books_table = Console_wrapper::Table::create_table(all_books_json);
        books_table
            ->include_only([year](auto&& j) { return !j["In library"] && j["Year"] <= year; })
            ->sort("Author")
            ->view();
    }

    void view_all_books() {
        Console_wrapper::Table::create_table(Book::get_json())
            ->sort("ID")
            ->view();
    }

    void search_book() {
        auto&& all_books_vector = Book::get_vector();
        if (all_books_vector.empty()) {
            Logger::Error("Список книг пока пуст!");
            return;
        }
        Console_wrapper::writeln("Введите название книги которую желаете найти");
        auto&& to_find = Console_wrapper::get_input<std::string>();
        for (auto&& book : all_books_vector) {
            if (std::string title = book.get_title(); title.contains(to_find)) {
                Console_wrapper::writeln("Книга найдена!");
                Console_wrapper::vec_write(book.get_data(), false);
                return;
            }
        }
        Logger::Error("Такой книги нет!");
    }

    void take_book() {
        auto&& all_books_json = Book::get_json();
        if (all_books_json.empty()) {
            Logger::Error("Список книг пока пуст!");
            return;
        }
        auto&& books_table = Console_wrapper::Table::create_table(all_books_json);
        auto&& included = books_table->include_only([](auto&& j) { return j["In library"]; });
        if (!included->get_sz())
            return;
        auto&& book = included->pick<Book>();
        User::get_current_user()->take_book(book);
        book.set_last_reader(User::get_current_user()->get_reader_ID());
        book.toggle_status();
        book.update_data();
        Logger::Succsess("Книга взята!");
    }

    void return_book() {
        auto&& taken_books = User::get_current_user()->get_taken_books();
        if (Book::get_json().empty() || taken_books.empty()) {
            Logger::Error("Нет взятых книг!");
            return;
        }
        Book book(Console_wrapper::vec_selection<std::string>(taken_books));
        User::get_current_user()->return_book(book);
        book.toggle_status();
        book.update_data();
        Logger::Succsess("Книга возвращена!");
    }

    void sort_books() {
        auto&& all_books_json = Book::get_json();
        if (all_books_json.empty()) {
            Logger::Error("Список книг пока пуст!");
            return;
        }
        auto&& books_table = Console_wrapper::Table::create_table(all_books_json);
        static const std::vector<std::string> choises = {"Название", "Автор", "Год выпуска", "ID книги",
                                                         "Издатель", "Количество страниц", "ID последнего читателя"};
        const auto choice = Console_wrapper::vec_selection<int16_t>(choises, true, "Выберите по чем сортировать:") + 1;

        Console_wrapper::writeln("Выберите способ сортировки:");
        Console_wrapper::writeln("1) От меньшего к большему");
        Console_wrapper::writeln("2) От большего к меньшему");
        const Sort_Method sm = Console_wrapper::get_inline_input<int16_t>() == 1 ? less : greater;
        switch (choice) {
            case 1:
                books_table->sort("Title", sm);
                break;
            case 2:
                books_table->sort("Author", sm);
                break;
            case 3:
                books_table->sort("Year", sm);
                break;
            case 4:
                books_table->sort("ID", sm);
                break;
            case 5:
                books_table->sort("Publisher", sm);
                break;
            case 6:
                books_table->sort("Pages", sm);
                break;
            case 7:
                books_table->sort("Last reader", sm);
                break;
        }
        books_table->view();
    }
}  // namespace USER_Functions

namespace ADMIN_Functions {
    void print_all_users() {
        auto&& all_users_json = User::get_json();
        if (all_users_json.empty()) {
            Logger::Error("Список пользователей пуст!");
            return;
        }
        auto&& user = Console_wrapper::Table::create_table(all_users_json)->pick<User>();
        Console_wrapper::draw_frame();
        Console_wrapper::writeln(std::format("Выбранный пользователь: {}", user.get_login()));
        Console_wrapper::vec_write(user.get_data(), false);
    }

    void add_book() {
        Console_wrapper::draw_frame("Добавление книги");
        Console_wrapper::write("Введите название: ");
        auto&& title_buf = Console_wrapper::get_inline_input<std::string>();
        Console_wrapper::write("Введите автора: ");
        auto&& author_buf = Console_wrapper::get_inline_input<std::string>();
        Console_wrapper::write("Введите издателя: ");
        auto&& pub_buf = Console_wrapper::get_inline_input<std::string>();
        Console_wrapper::write("Введите количество страниц: ");
        auto&& pages_buf = Console_wrapper::get_inline_input<uint16_t>();
        Console_wrapper::write("Введите год выпуска: ");
        auto&& year_buf = Console_wrapper::get_inline_input<uint16_t>();
        Console_wrapper::writeln("Выберите статус: ");
        Console_wrapper::writeln("1) В библиотеке");
        Console_wrapper::writeln("2) У читателя");
        auto&& in_lib_buf = Console_wrapper::get_inline_input<uint16_t>() == 1;
        Book(title_buf, in_lib_buf, year_buf, pages_buf, author_buf, pub_buf);
        Logger::Succsess("Книга успешно добавлена!");
    }

    void add_user() {
        Console_wrapper::draw_frame("Добавление пользователя");
        auto&& all_users_json = User::get_json();
        Console_wrapper::write("Введите логин: ");
        auto&& login_buf = Console_wrapper::get_inline_input<std::string>();
        if (all_users_json.contains(login_buf)) {
            Logger::Error("Аккаунт с таким логином уже существует");
            std::cin.get();
            return;
        }
        Console_wrapper::write("Задайте пароль: ");
        auto&& passw_buf = Console_wrapper::get_inline_input<std::string>(true);
        Console_wrapper::writeln("Выберите роль:");
        Console_wrapper::writeln("1) Администратор");
        Console_wrapper::writeln("2) Пользователь");
        auto&& ur = Console_wrapper::get_inline_input<int16_t>() == 1 ? User_role::admin : User_role::user;
        User(login_buf, passw_buf, ur);
        Logger::Succsess("Пользователь успешно добавлен!");
    }

    void edit_user() {
        auto&& all_users_json = User::get_json();
        if (all_users_json.empty()) {
            Logger::Error("Список пользователей пуст!");
            return;
        }
        auto&& user = Console_wrapper::Table::create_table(all_users_json)->pick<User>();

        Console_wrapper::draw_frame();
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
                Console_wrapper::writeln("Статус была изменена!");
                user.set_role(User_role(!bool(user.get_role())));
                break;
            default:
                return;
        }
        user.update_data();
        Console_wrapper::writeln("Новые данные сохранены");
    }

    void erase_user() {
        auto&& all_users_json = User::get_json();
        if (all_users_json.empty()) {
            Logger::Error("Список пользователей пуст!");
            return;
        }
        auto&& user = Console_wrapper::Table::create_table(all_users_json)->pick<User>();
        User::erase(user.get_login());
    }

    void remove_file() {
        std::vector<std::string> files;
        for (auto&& f : std::filesystem::directory_iterator(std::filesystem::current_path())) {
            if (const auto& f_path = f.path(); f_path.extension() == ".json")
                files.emplace_back(f_path.filename().string());
        }
        Console_wrapper::writeln("Выберите файл");
        auto&& to_remove = Console_wrapper::vec_selection<std::string>(files);
        Console_wrapper::writeln(std::format("Вы уверены, что хотите удалить {}?", to_remove));
        Console_wrapper::writeln("1) Да");
        Console_wrapper::writeln("2) Нет");
        if (Console_wrapper::get_input<uint16_t>() == 1) {
            std::filesystem::remove(to_remove);
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

struct FUNCTION {
    const char* name;
    void (*func)();
    constexpr void invoke() const { func(); }
};

class User_lib : public virtual ILibrary {
   protected:
    static inline const std::vector<FUNCTION> user_funcs{
        {"задание по варианту", USER_Functions::my_task},
        {"просмотреть все книги", USER_Functions::view_all_books},
        {"поиск книги", USER_Functions::search_book},
        {"взять книгу", USER_Functions::take_book},
        {"вернуть книгу", USER_Functions::return_book},
        {"сортировка", USER_Functions::sort_books},  //
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
        {"добавить книгу", ADMIN_Functions::add_book},
        {"добавить учетную запись", ADMIN_Functions::add_user},
        {"отредактировать учетную запись", ADMIN_Functions::edit_user},
        {"удалить учетную запись", ADMIN_Functions::erase_user},
        {"удалить файл", ADMIN_Functions::remove_file},
    };

   public:
    [[nodiscard]] constexpr std::vector<std::string> get_menu() const override {
        auto&& menu = User_lib::get_menu();
        for (auto&& f : admin_funcs)
            menu.emplace_back(f.name);
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
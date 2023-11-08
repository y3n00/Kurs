#pragma once

#include <format>
#include <ranges>
#include <sstream>
#include <vector>

#include "Console_wrapper.hpp"

constexpr static auto MENU_ITEM_FMT = "{: >2}) {}\n";

struct FUNCTION {
    const char* name;
    void (*func)();
    constexpr void invoke() const { func(); }
};

class Library_as_user {
   public:
    static inline const std::vector<FUNCTION> user_funcs{
        {"просмотреть все данные", [] { Console_wrapper::writeln("user #1"); }},
        {"поиск данных", [] { Console_wrapper::writeln("user #2"); }},
        {"сортировка", [] { Console_wrapper::writeln("user #3"); }},
    };

    [[nodiscard]] virtual std::stringstream get_menu() const {
        std::stringstream sstr;

        for (const auto& [idx, FUNCTION] : user_funcs | std::views::enumerate)
            sstr << std::format(MENU_ITEM_FMT, idx + 1, FUNCTION.name);

        return sstr;
    }

    virtual void do_at(uint16_t idx) const {
        user_funcs.at(idx).invoke();
    }

    [[nodiscard]] virtual size_t get_menu_size() const {
        return user_funcs.size();
    }

    virtual ~Library_as_user() = default;
};

class Library_as_admin : virtual public Library_as_user {
   public:
    static inline const std::vector<FUNCTION> admin_funcs{
        {"просмотреть все учетные записи", [] { Console_wrapper::writeln("admin #1"); }},
        {"добавить учетную запись", [] { Console_wrapper::writeln("admin #2"); }},
        {"отредактировать учетную запись", [] { Console_wrapper::writeln("admin #3"); }},
        {"удалить учетную запись", [] { Console_wrapper::writeln("admin #4"); }},
    };

    [[nodiscard]] std::stringstream get_menu() const override {
        const auto menu_delta = Library_as_user::get_menu_size();
        auto sstr = Library_as_user::get_menu();

        for (const auto& [idx, FUNCTION] : admin_funcs | std::views::enumerate)
            sstr << std::format(MENU_ITEM_FMT, idx + menu_delta + 1, FUNCTION.name);
        return sstr;
    }

    void do_at(uint16_t idx) const override {
        const auto user_menu_size = Library_as_user::get_menu_size();
        if (idx <= user_menu_size)
            Library_as_user::do_at(idx);
        else
            admin_funcs.at(idx - user_menu_size).invoke();
    }

    [[nodiscard]] size_t get_menu_size() const override {
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

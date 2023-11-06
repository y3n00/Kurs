#pragma once

#include <functional>
#include <map>
#include <print>
#include <ranges>
#include <sstream>
#include <utility>

#include "Console_wrapper.hpp"

constexpr static auto MENU_ITEM_FMT = "{: >2}) {}\n";

struct FUNCDEF {
    const char* name;
    void (*func)();
    inline void invoke() const { func(); }
};

class Library_as_user {
   public:
    static inline const std::map<size_t, FUNCDEF> user_funcs{
        {1, {"просмотреть все данные", [] { Console_wrapper::writeln("user #1"); }}},
        {2, {"поиск данных", [] { Console_wrapper::writeln("user #2"); }}},
        {3, {"сортировка", [] { Console_wrapper::writeln("user #3"); }}},
    };

    [[nodiscard]] virtual std::stringstream get_menu() const {
        std::stringstream sstr;

        for (const auto& [idx, funcdef] : user_funcs)
            sstr << std::format(MENU_ITEM_FMT, idx, funcdef.name);

        return sstr;
    }

    virtual void do_at(int idx) const {
        user_funcs.at(idx).invoke();
    }

    [[nodiscard]] virtual size_t get_menu_size() const {
        return user_funcs.size();
    }

    virtual ~Library_as_user() = default;
};

class Library_as_admin : virtual public Library_as_user {
   public:
    static inline const std::map<size_t, FUNCDEF> admin_funcs{
        {4, {"просмотреть все учетные записи", [] { Console_wrapper::writeln("admin #1"); }}},
        {5, {"добавить учетную запись", [] { Console_wrapper::writeln("admin #2"); }}},
        {6, {"отредактировать учетную запись", [] { Console_wrapper::writeln("admin #3"); }}},
        {7, {"удалить учетную запись", [] { Console_wrapper::writeln("admin #4"); }}},
    };

    [[nodiscard]] std::stringstream get_menu() const override {
        auto sstr = Library_as_user::get_menu();

        for (const auto& [idx, funcdef] : admin_funcs)
            sstr << std::format(MENU_ITEM_FMT, idx, funcdef.name);
        return sstr;
    }

    void do_at(int idx) const override {
        if (idx <= Library_as_user::get_menu_size())
            Library_as_user::do_at(idx);
        else
            admin_funcs.at(idx).invoke();
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

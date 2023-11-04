#pragma once

#include <functional>
#include <map>
#include <print>
#include <ranges>
#include <sstream>
#include <utility>

#include "Log.hpp"

constexpr static auto menu_item_fmt = "{: >2}){}\n";

class Library_as_user {
   public:
    static inline const std::vector<std::string> user_lib_menu{
        "просмотреть все данные",
        "поиск данных",
        "сортировка",
    };

    virtual std::stringstream get_menu() const {
        std::stringstream sstr;
        sstr << std::format("{0:-^50}\n", "This is user menu");

        for (const auto& [idx, item] : user_lib_menu | std::views::enumerate) {
            sstr << std::format(menu_item_fmt, idx + 1, item);
        }
        return sstr;
    }
    virtual void do_at(int idx) {}

    [[nodiscard]] virtual size_t get_menu_size() const { return user_lib_menu.size(); }

    virtual ~Library_as_user() = default;
};

class Library_as_admin : virtual public Library_as_user {
   public:
    static inline const std::vector<std::string> admin_lib_menu{
        "просмотреть все учетные записи",
        "добавить учетную запись",
        "отредактировать учетную запись",
        "удалить учетную запись",
    };

    std::stringstream get_menu() const override {
        auto sstr = Library_as_user::get_menu();
        sstr << std::format("{0:-^50}\n", "This is admin menu");

        const size_t delta_size = Library_as_user::user_lib_menu.size() + 1;
        for (const auto& [idx, item] : user_lib_menu | std::views::enumerate)
            sstr << std::format(menu_item_fmt, idx + delta_size, item);
        return sstr;
    }
    void do_at(int idx) override {}

    [[nodiscard]] size_t get_menu_size() const override { return admin_lib_menu.size(); }
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

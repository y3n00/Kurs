#pragma once

#include <functional>
#include <map>
#include <print>
#include <ranges>
#include <utility>

#include "Log.hpp"

class Library_as_user {
   public:
    static inline const std::vector<std::string> user_lib_menu{
        "просмотреть все данные",
        "поиск данных",
        "сортировка",
    };

    virtual void print_menu() {
        std::println("\n\n{0:-^50}", "This is user menu");

        for (const auto& [idx, item] : user_lib_menu | std::views::enumerate) {
            std::println("{: >2}){}", idx + 1, item);
        }
    }

    virtual void main_loop() const {}
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

    void print_menu() override {
        Library_as_user::print_menu();

        std::println("{0:-^50}", "This is admin menu");

        const size_t delta_size = Library_as_user::user_lib_menu.size() + 1;
        for (const auto& [idx, item] : user_lib_menu | std::views::enumerate) {
            Logger::Warning(std::format("{: >2}){}", idx + delta_size, item));
        }
    }

    void main_loop() const override {
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

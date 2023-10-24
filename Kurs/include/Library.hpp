#pragma once

#include <print>

class Library_as_user {
   protected:
    virtual void print_menu() {
        std::println("{0:-^50}", "This is user menu");
    }
    virtual void menu_input(int task_idx) {}

   public:
    virtual void logic() {
    }
    virtual ~Library_as_user() = default;
};

class Library_as_admin : virtual public Library_as_user {
    void print_menu() override {
        Library_as_user::print_menu();
        std::println("{0:-^50}", "This is admin menu");
    }

   public:
    void logic() override {
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

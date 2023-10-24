#pragma once

#include <print>

class Library_as_user {
   public:
    virtual void menu() {
        std::println("{0:-^50}", "This is user menu");
    }
    virtual ~Library_as_user() = default;
};

class Library_as_admin : virtual public Library_as_user {
   public:
    void menu() override {
        Library_as_user::menu();
        std::println("{0:-^50}", "This is admin menu");
    }
};

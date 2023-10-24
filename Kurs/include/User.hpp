#pragma once

#include <iostream>
#include <stacktrace>
#include <string>
#include <utility>

#include "Fsystem.hpp"
#include "thirdparty/json.hpp"

enum User_role {
    admin = 0,
    user
};

template <typename T>
[[nodiscard]] constexpr auto encrypt(T arg) {
    return std::hash<T>{}(arg);
}

class User {
   private:
    static inline size_t global_user_id = 1;
    static inline nlohmann::json all_accs;

   public:
    static void load_accounts(std::string_view filename) {
        FileSystem::load(filename, all_accs);
    }
    [[nodiscard]] static const auto& get_all_accs() { return all_accs; }

   private:
    User_role user_role{};
    size_t user_id{global_user_id++}, user_encrypted_passw{};
    std::string user_login{};
    inline void to_global_json() const {
        using namespace nlohmann;
        json& j = all_accs[user_login];
        j["Role"] = user_role;
        j["Password"] = user_encrypted_passw;
        j["ID"] = user_id;
    }

   public:
    User(std::string_view login,
         const nlohmann::json& acc_data) : user_role{acc_data["Role"].get<User_role>()},
                                           user_id{acc_data["ID"].get<size_t>()},
                                           user_encrypted_passw{acc_data["Password"].get<size_t>()},
                                           user_login{login} {}

    User(std::string_view login,
         std::string_view passw,
         User_role role) : user_role{role},
                           user_encrypted_passw{encrypt(passw)},
                           user_login{login} {
        to_global_json();
    }

    User(User&& other_user) : user_role{other_user.user_role},
                              user_id{other_user.user_id},
                              user_encrypted_passw{other_user.user_encrypted_passw},
                              user_login{other_user.user_login} {}

    ~User() = default;

    [[nodiscard]] auto is_admin() const { return user_role == User_role::admin; }
    [[nodiscard]] auto get_role() const { return user_role; }
    [[nodiscard]] auto get_reader_ID() const { return user_id; }
    [[nodiscard]] auto get_formated_ID() const { return std::format("{0:0>6}", user_id); }
    [[nodiscard]] auto get_passw() const { return user_encrypted_passw; }
    [[nodiscard]] auto get_login() const { return user_login; }
};

namespace {
[[nodiscard]] static User login() {
    const auto con_sz = Console::getSizeByChars();   // Header
    std::println("{:=^{}}", "Логин", con_sz.width);  //

    std::string login_buf, passw_buf;
    std::print("{}", "Введите логин: ");
    std::getline(std::cin, login_buf);
    if (!User::get_all_accs().contains(login_buf)) {
        Logger::Error("Аккаунта с таким логином несуществует");
        return login();
    }
    const auto& user_js = User::get_all_accs().at(login_buf);

    std::print("{}", "Введите пароль: ");
    std::getline(std::cin, passw_buf);
    if (user_js.at("Password").get<std::size_t>() != encrypt(passw_buf)) {
        Logger::Error("Неверный пароль");
        return login();
    }

    return User(login_buf, user_js);
}

[[nodiscard]] static User registration() {
    const auto con_sz = Console::getSizeByChars();         // Header
    std::println("{:=^{}}", "Регистрация", con_sz.width);  //

    int role_buf;
    std::string login_buf, passw_buf, rep_passw_buf;

    std::print("{}", "Введите логин: ");
    std::getline(std::cin, login_buf);
    if (User::get_all_accs().contains(login_buf)) {
        Logger::Error("Аккаунт с таким логином уже существует");
        return registration();
    }

    std::print("{}", "Введите пароль: ");
    std::getline(std::cin, passw_buf);

    std::print("{}", "Повторите пароль: ");
    std::getline(std::cin, rep_passw_buf);
    if (passw_buf != rep_passw_buf) {
        Logger::Error("Пароли не совпадают");
        return registration();
    }

    std::print("{}", "Выберите роль:\n1)Администратор\n2)Пользователь\n");
    std::cin >> role_buf;

    const auto role = static_cast<User_role>(std::clamp(role_buf - 1, 0, 1));
    return User(login_buf, passw_buf, role);
}
}  // namespace

User authorize() {
    return User::get_all_accs().empty() ? registration() : login();
}
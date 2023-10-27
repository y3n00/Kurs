#pragma once

#include <iostream>
#include <string>
#include <utility>

#include "Fsystem.hpp"
#include "thirdparty/json.hpp"

enum User_role : uint16_t {
    admin = 0,
    user
};

[[nodiscard]] auto encrypt_str(std::string_view arg, size_t key) {
    return std::hash<std::string>{}(arg |
                                    std::views::transform([key](char ch) { return ch + key; }) |
                                    std::ranges::to<std::string>());
}

class User {
   private:
    static inline size_t next_user_id = 1;
    static inline nlohmann::json all_accs;

   public:
    static void load_accounts(std::string_view filename) {
        FileSystem::load(filename, all_accs);
        if (all_accs.empty()) {
            next_user_id = 1;
        } else {
            for (const auto& [_, data] : all_accs.items())
                next_user_id = std::max(next_user_id, data.at("ID").get<size_t>());
            next_user_id += 1;
        }
    }
    [[nodiscard]] static const auto& get_all_accs() { return all_accs; }

   private:
    User_role user_role{};
    size_t user_id{next_user_id++}, user_encrypted_passw{};
    std::string user_login{};

    inline void to_global_json() const {
        auto& j = all_accs[user_login];
        j["Role"] = user_role;
        j["Password"] = user_encrypted_passw;
        j["ID"] = user_id;
    }

   public:
    User(std::string_view login,
         const nlohmann::json& acc_data) : user_role{acc_data.at("Role").get<User_role>()},
                                           user_id{acc_data.at("ID").get<size_t>()},
                                           user_encrypted_passw{acc_data.at("Password").get<size_t>()},
                                           user_login{login} {}

    User(std::string_view login,
         std::string_view passw,
         User_role role) : user_role{role},
                           user_encrypted_passw{encrypt_str(passw, login.length())},
                           user_login{login} {
        to_global_json();
    }

    User(User&& other_user) noexcept : user_role{other_user.user_role},
                                       user_id{other_user.user_id},
                                       user_encrypted_passw{other_user.user_encrypted_passw},
                                       user_login{other_user.user_login} {}

    ~User() = default;

    [[nodiscard]] auto is_admin() const { return user_role == User_role::admin; }
    [[nodiscard]] auto get_formated_ID() const { return std::format("{0:0>6}", user_id); }

    [[nodiscard]] auto get_role() const { return user_role; }
    inline void set_role(const auto& new_value) { user_role = new_value; }

    [[nodiscard]] auto get_reader_ID() const { return user_id; }
    inline void update_ID(const auto& new_value) { user_id = next_user_id++; }

    [[nodiscard]] auto get_passw() const { return user_encrypted_passw; }
    inline void set_password(const auto& new_value) {
        user_encrypted_passw = encrypt_str(new_value, user_login.length());
    }

    [[nodiscard]] auto get_login() const { return user_login; }
    inline void set_login(const auto& new_value) { user_login = new_value; }
};

namespace {
[[nodiscard]] static User registration() {
    const auto con_sz = Console::getSizeByChars();         // Header
    std::println("{:=^{}}", "Регистрация", con_sz.width);  //

    int role_buf;
    std::string login_buf, passw_buf, rep_passw_buf;

    std::print("{}", "Введите логин: ");
    std::cin.ignore();
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

[[nodiscard]] static User login() {
    const auto con_sz = Console::getSizeByChars();   // Header
    std::println("{:=^{}}", "Логин", con_sz.width);  //

    std::string login_buf, passw_buf;
    std::print("{}", "Введите логин: ");
    std::cin.ignore();
    std::getline(std::cin, login_buf);
    if (not User::get_all_accs().contains(login_buf)) {
        Logger::Error("Аккаунта с таким логином несуществует");
        return login();
    }
    const auto& user_data = User::get_all_accs().at(login_buf);
    const auto user_passw = user_data.at("Password").get<std::size_t>();

    std::print("{}", "Введите пароль: ");
    std::getline(std::cin, passw_buf);
    if (user_passw not_eq encrypt_str(passw_buf, login_buf.length())) {
        Logger::Error("Неверный пароль");
        return login();
    }

    return User(login_buf, user_data);
}
}  // namespace

[[nodiscard]] User authorize() {
    if (User::get_all_accs().empty())
        return registration();

    int input_buf;
    std::println("{}\n{}\n{}", "Хотите создать новый аккаунт?", "1)Да", "2)Нет");
    std::cin >> input_buf;
    return std::clamp(input_buf - 1, 0, 1) == 0 ? registration() : login();
}
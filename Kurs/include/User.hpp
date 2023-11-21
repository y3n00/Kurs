#pragma once

#include <print>
#include <string>
#include <utility>

#include "Book.hpp"
#include "Console.hpp"
#include "Console_wrapper.hpp"
#include "Fsystem.hpp"
#include "Log.hpp"
#include "thirdparty/json.hpp"

enum User_role : uint16_t {
    admin = 0,
    user
};

[[nodiscard]] size_t encrypt_str(std::string_view arg, size_t key) {  // caesar + hash
    return std::hash<std::string>{}(arg |
                                    std::views::transform([key](char ch) { return ch + key; }) |
                                    std::ranges::to<std::string>());
}

class User {
   private:
    static inline size_t next_user_id = 1;
    static inline nlohmann::json all_accs{};

   public:
    static inline std::unique_ptr<User> current_global_user;
    static void load_accounts(std::string_view filename) {
        FileSystem::load(filename, all_accs);
        if (!all_accs.empty()) {
            for (auto&& [_, data] : all_accs.items())
                next_user_id = std::max(next_user_id, data.at("ID").get<size_t>());
            next_user_id += 1;
        }
    }
    [[nodiscard]] static const auto& get_all_accs() { return all_accs; }
    [[nodiscard]] static auto& get_current_user() { return current_global_user; }
    static void erase(std::string_view user_login) { all_accs.erase(user_login); }

   private:
    User_role user_role{};
    size_t user_id{next_user_id++}, user_encrypted_passw{};
    std::string user_login{}, passw_raw_data;

   public:
    User(std::string_view login)
        : user_login{login} {
        const nlohmann::json& CURRENT_USER_DATA = all_accs.at(user_login);
        CURRENT_USER_DATA["Role"].get_to(user_role);
        CURRENT_USER_DATA["Password"].get_to(user_encrypted_passw);
        CURRENT_USER_DATA["ID"].get_to(user_id);

        update_data();
    }

    User(std::string_view login, std::string_view passw, User_role ROLE)
        : user_role{ROLE},
          passw_raw_data{passw},
          user_login{login} {
        set_password(passw_raw_data);
        update_data();
    }

    User(User&& other_user) noexcept
        : user_role{other_user.user_role},
          user_id{other_user.user_id},
          user_encrypted_passw{other_user.user_encrypted_passw},
          user_login{other_user.user_login} {}

    ~User() = default;

    [[nodiscard]] auto is_admin() const { return user_role == User_role::admin; }
    [[nodiscard]] auto get_formatted_ID() const { return std::format("{:0>6}", user_id); }

    [[nodiscard]] auto get_role() const { return user_role; }
    inline void set_role(const auto& new_value) { user_role = new_value; }

    [[nodiscard]] auto get_reader_ID() const { return user_id; }
    inline void update_ID(const auto& new_value) { user_id = next_user_id++; }

    [[nodiscard]] auto get_passw() const { return user_encrypted_passw; }
    inline void set_password(const auto& new_value) {
        passw_raw_data = new_value;
        user_encrypted_passw = encrypt_str(passw_raw_data, user_login.length());
    }

    [[nodiscard]] auto get_login() const { return user_login; }
    inline void set_login(const auto& new_value) {
        all_accs.erase(user_login);
        user_login = new_value;
        set_password(passw_raw_data);  // rehash current passw after login change
    }

    [[nodiscard]] auto get_data() const {
        return std::vector<std::string>{
            std::format("Логин: {}", user_login),
            std::format("Хеш пароля: {}", user_encrypted_passw),
            std::format("Статус: {}", (user_role == User_role::admin ? "админ" : "пользователь")),
            std::format("Взято книг: {}", all_accs[user_login]["Taken books"].size()),
        };
    }

    void update_data() const {
        auto& j = all_accs[user_login];
        j["Role"] = user_role;
        j["Password"] = user_encrypted_passw;
        j["ID"] = user_id;
    }

    void take_book(Book& book) const {
        book.set_last_reader(get_reader_ID());
        book.toggle_status();
        all_accs[user_login]["Taken books"] += book.get_title();
    }

    void return_book(Book& book) {
        book.toggle_status();
        const std::string TITLE = book.get_title();
        nlohmann::json& taken_books = all_accs[user_login]["Taken books"];
        for (auto&& it = taken_books.begin(); it != taken_books.end(); ++it) {
            if (it->get<std::string>() == TITLE) {
                taken_books.erase(it);
                break;
            }
        }
    }

    auto get_taken_books() const {
        return all_accs[user_login]["Taken books"] |
               std::views::transform([](auto&& js_obj) { return js_obj.get<std::string>(); }) |
               std::ranges::to<std::vector<std::string>>();
    }
};

namespace AUTHORIZATION_FROMS {
    [[nodiscard]] std::unique_ptr<User>& login_form() {
        Console_wrapper::draw_frame(" Логин ");
        const nlohmann::json& ALL_ACCS = User::get_all_accs();

        Console_wrapper::write("Введите логин: ");
        const std::string LOGIN_BUF = Console_wrapper::get_inline_input<std::string>();
        if (!ALL_ACCS.contains(LOGIN_BUF)) {
            Logger::Error("Аккаунта с таким логином несуществует");
            std::cin.get();
            return login_form();
        }
        const size_t USER_PASSW = ALL_ACCS[LOGIN_BUF].at("Password").get<std::size_t>();

        Console_wrapper::write("Введите пароль: ");
        const std::string PASSW_BUF = Console_wrapper::get_inline_input<std::string>(true);
        if (USER_PASSW != encrypt_str(PASSW_BUF, LOGIN_BUF.length())) {
            Logger::Error("Неверный пароль");
            std::cin.get();
            return login_form();
        }
        User::current_global_user = std::make_unique<User>(LOGIN_BUF);
        return User::get_current_user();
    }

    [[nodiscard]] std::unique_ptr<User>& registration_form() {
        Console_wrapper::draw_frame(" Регистрация ");
        const nlohmann::json& ALL_ACCS = User::get_all_accs();

        Console_wrapper::write("Введите логин: ");
        const std::string LOGIN_BUF = Console_wrapper::get_inline_input<std::string>();
        if (ALL_ACCS.contains(LOGIN_BUF)) {
            Logger::Error("Аккаунт с таким логином уже существует");
            std::cin.get();
            return registration_form();
        }

        Console_wrapper::write("Введите пароль: ");
        const std::string PASSW_BUF = Console_wrapper::get_inline_input<std::string>(true);

        Console_wrapper::write("Повторите пароль: ");
        const std::string REP_PASSW_BUF = Console_wrapper::get_inline_input<std::string>(true);
        if (PASSW_BUF != REP_PASSW_BUF) {
            Logger::Error("Пароли не совпадают");
            std::cin.get();
            return registration_form();
        }

        Console_wrapper::writeln("Выберите роль:");
        Console_wrapper::writeln("1) Администратор");
        Console_wrapper::writeln("2) Пользователь");
        const int16_t temp_value = Console_wrapper::get_inline_input<int16_t>();
        const User_role ROLE = temp_value == 1 ? User_role::admin : User_role::user;
        User::current_global_user = std::make_unique<User>(LOGIN_BUF, PASSW_BUF, ROLE);
        return User::get_current_user();
    }
}  // namespace AUTHORIZATION_FROMS

[[nodiscard]] auto& authorize() {
    auto&& force_reg = []() -> bool {
        if (User::get_all_accs().empty())
            return true;
        Console_wrapper::draw_frame(" Авторизация ");
        Console_wrapper::writeln("Cоздать новый аккаунт?");
        Console_wrapper::writeln("1) Да");
        Console_wrapper::writeln("2) Нет");
        return Console_wrapper::get_input<int>() == 1;
    };
    return force_reg() ? AUTHORIZATION_FROMS::registration_form() : AUTHORIZATION_FROMS::login_form();
}
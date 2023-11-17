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

constexpr static auto HEADER_FMT = "{:=^{}}";

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
            for (const auto& [_, data] : all_accs.items())
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
    std::string user_login{};

   public:
    User(std::string_view login, const nlohmann::json& acc_data)
        : user_role{acc_data.at("Role").get<User_role>()},
          user_id{acc_data.at("ID").get<size_t>()},
          user_encrypted_passw{acc_data.at("Password").get<size_t>()},
          user_login{login} {
        update_data();
    }

    User(std::string_view login, std::string_view passw, User_role ROLE)
        : user_role{ROLE},
          user_encrypted_passw{encrypt_str(passw, login.length())},
          user_login{login} {
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
        user_encrypted_passw = encrypt_str(new_value, user_login.length());
    }

    [[nodiscard]] auto get_login() const { return user_login; }
    inline void set_login(const auto& new_value) {
        all_accs.erase(user_login);
        user_login = new_value;
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

    void take_book(Book& book) {
        book.set_last_reader(get_reader_ID());
        book.change_status();
        all_accs[user_login]["Taken books"] += book.get_title();
    }

    void return_book(Book& book) {
        book.change_status();
        const auto& title = book.get_title();
        auto&& taken_books = all_accs[user_login]["Taken books"];
        for (auto it = taken_books.begin(); it != taken_books.end(); ++it) {
            if (it->get<std::string>() == title) {
                taken_books.erase(it);
                break;
            }
        }
    }

    auto get_book_list() const {
        return all_accs[user_login]["Taken books"] |
               std::views::transform([](auto&& js_obj) { return js_obj.get<std::string>(); }) |
               std::ranges::to<std::vector<std::string>>();
    }
};

namespace {
[[nodiscard]] static auto enter_password() {
    std::string passw;
    char inp;

    while (true) {
        inp = _getch();

        switch (inp) {
            case Keys::ENTER:
                std::cout << '\n';
                return passw;
            case Keys::BACKSPACE:
                if (!passw.empty()) {
                    passw.pop_back();
                    std::cout << "\b \b";
                }
                continue;
        }
        passw.push_back(inp);
        std::cout << '*';
    }
}

[[nodiscard]] static std::unique_ptr<User> registration() {
    std::println(HEADER_FMT, " Регистрация ", Console::getSizeByChars().width);  // Header

    int16_t role_buf;
    std::string login_buf;

    std::print("Введите логин: ");
    std::getline(std::cin, login_buf);
    if (User::get_all_accs().contains(login_buf)) {
        Logger::Error("Аккаунт с таким логином уже существует");
        return registration();
    }

    std::print("Введите пароль: ");
    const auto PASSW_BUF = enter_password();

    std::print("Повторите пароль: ");
    const auto REP_PASSW_BUF = enter_password();

    if (PASSW_BUF != REP_PASSW_BUF) {
        Logger::Error("Пароли не совпадают");
        return registration();
    }

    std::print("Выберите роль:\n1) Администратор\n2) Пользователь\n");
    (std::cin >> role_buf).get();

    const User_role ROLE = role_buf - 1 ? User_role::user : User_role::admin;
    return std::make_unique<User>(login_buf, PASSW_BUF, ROLE);
}

[[nodiscard]] static std::unique_ptr<User> login() {
    std::println(HEADER_FMT, " Логин ", Console::getSizeByChars().width);  // Header

    std::string login_buf;
    std::print("Введите логин: ");
    std::getline(std::cin, login_buf);

    if (!User::get_all_accs().contains(login_buf)) {
        Logger::Error("Аккаунта с таким логином несуществует");
        return login();
    }
    const auto& USER_DATA = User::get_all_accs().at(login_buf);
    const auto USER_PASSW = USER_DATA.at("Password").get<std::size_t>();

    std::print("Введите пароль: ");
    const auto PASSW_BUF = enter_password();

    if (USER_PASSW != encrypt_str(PASSW_BUF, login_buf.length())) {
        Logger::Error("Неверный пароль");
        return login();
    }

    return std::make_unique<User>(login_buf, USER_DATA);
}
}  // namespace

[[nodiscard]] std::unique_ptr<User> authorize() {
    if (User::get_all_accs().empty())
        return registration();

    int16_t input_buf;
    std::println("Хотите создать новый аккаунт?\n1) Да\n2) Нет");
    (std::cin >> input_buf).get();
    return input_buf == 1 ? registration() : login();
}
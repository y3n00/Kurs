#pragma once
#include <format>
#include <iostream>
#include <memory>
#include <print>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include "Book.hpp"
#include "Console_wrapper.hpp"
#include "Fsystem.hpp"
#include "Log.hpp"
#include "thirdparty/json.hpp"

enum User_role : uint16_t {
  admin = 0,
  user
};

class User {
 private:
  static inline size_t next_user_id = 1;
  static inline nlohmann::json users_json{};
  static inline std::unique_ptr<User> current_global_user;

 public:
  static void load_accounts(std::string_view filename) {
    FileSystem::load(filename, users_json);
    if (!users_json.empty() && !users_json.is_null()) {
      for (auto&& [_, data] : users_json.items())
        next_user_id =
            std::max(next_user_id, data.at("ID").get<size_t>());
      next_user_id += 1;
    }
  }
  [[nodiscard]] static const auto& get_json() { return users_json; }
  [[nodiscard]] static auto& get_current_user() { return current_global_user; }
  static void erase(std::string_view user_login) { users_json.erase(user_login); }
  [[nodiscard]] static std::vector<User> get_vector() {
    return users_json.items() |
           std::views::transform([](auto&& json_item) {
             return User(json_item.key());
           }) |
           std::ranges::to<std::vector<User>>();
  }

 private:
  User_role user_role;
  size_t user_id{next_user_id++}, user_encrypted_passw;
  std::string user_login, passw_raw_data;

 public:
  User(std::string_view login) : user_login{login} {
    const nlohmann::json& CURRENT_USER_DATA =
        users_json.at(user_login);
    CURRENT_USER_DATA["Role"].get_to(user_role);
    CURRENT_USER_DATA["Password"].get_to(user_encrypted_passw);
    CURRENT_USER_DATA["ID"].get_to(user_id);
    update_data();
  }

  User(std::string_view login, std::string_view passw, User_role ROLE)
      : user_role{ROLE}, user_login{login} {
    set_password(passw);
    update_data();
  }

  User(User&& other_user) noexcept
      : user_role{other_user.user_role},
        user_id{other_user.user_id},
        user_encrypted_passw{other_user.user_encrypted_passw},
        user_login{other_user.user_login} {}

  ~User() = default;

  [[nodiscard]] auto is_admin() const { return user_role == User_role::admin; }
  [[nodiscard]] auto get_role() const { return user_role; }
  [[nodiscard]] auto get_reader_ID() const { return user_id; }
  [[nodiscard]] auto get_passw() const { return user_encrypted_passw; }
  [[nodiscard]] auto get_login() const { return user_login; }

  inline void set_role(const auto& new_value) { user_role = new_value; }
  inline void update_ID(const auto& new_value) { user_id = next_user_id++; }
  inline void set_password(const auto& new_value) {
    passw_raw_data = new_value;
    user_encrypted_passw = encrypt_str(passw_raw_data, user_login.length());
  }
  inline void set_login(const auto& new_value) {
    users_json.erase(user_login);
    user_login = new_value;
    set_password(passw_raw_data);  // rehash current passw after login change
    update_data();
  }

  [[nodiscard]] auto get_data() const {
    return std::vector<std::string>{
        std::format("Логин: {}", user_login),
        std::format("Хеш пароля: {}", user_encrypted_passw),
        std::format("Статус: {}", (user_role == User_role::admin ? "админ" : "пользователь")),
        std::format("Взято книг: {}", users_json[user_login]["Taken books"].size()),
    };
  }

  void update_data() const {
    auto& j = users_json[user_login];
    j["Role"] = user_role;
    j["Password"] = user_encrypted_passw;
    j["ID"] = user_id;
  }

  void take_book(Book& book) const {
    users_json[user_login]["Taken books"] += book.get_title();
  }

  void return_book(Book& book) {
    auto&& title = book.get_title();
    auto& taken_books = users_json[user_login]["Taken books"];
    for (auto&& it = taken_books.begin(); it != taken_books.end(); ++it) {
      if (it->get<std::string>() == title) {
        taken_books.erase(it);
        break;
      }
    }
  }

  [[nodiscard]] std::vector<std::string> get_taken_books() const {
    auto&& taken_books = users_json[user_login]["Taken books"];
    if (taken_books.empty() || taken_books.is_null())
      return {};
    return taken_books |
           std::views::transform([](auto&& book_title_js) {
             return book_title_js.get<std::string>();
           }) |
           std::ranges::to<std::vector<std::string>>();
  }
};

namespace AUTHORIZATION_FORMS {
  [[nodiscard]] std::unique_ptr<User>& login_form() {
    Console_wrapper::draw_frame(" Логин ");
    const nlohmann::json& ALL_ACCS = User::get_json();

    Console_wrapper::write("Введите логин: ");
    const auto LOGIN_BUF = Console_wrapper::get_inline_input<std::string>();
    if (!ALL_ACCS.contains(LOGIN_BUF)) {
      Logger::Error("Аккаунта с таким логином не существует");
      std::cin.get();
      return login_form();
    }
    const size_t USER_PASSW = ALL_ACCS[LOGIN_BUF].at("Password").get<std::size_t>();

    Console_wrapper::write("Введите пароль: ");
    const auto PASSW_BUF = Console_wrapper::get_inline_input<std::string>(true);
    if (USER_PASSW != encrypt_str(PASSW_BUF, LOGIN_BUF.length())) {
      Logger::Error("Неверный пароль");
      std::cin.get();
      return login_form();
    }
    User::get_current_user() = std::make_unique<User>(LOGIN_BUF);
    User::get_current_user()->set_password(PASSW_BUF);
    return User::get_current_user();
  }

  [[nodiscard]] std::unique_ptr<User>& registration_form() {
    Console_wrapper::draw_frame(" Регистрация ");
    const nlohmann::json& ALL_ACCS = User::get_json();

    Console_wrapper::write("Введите логин: ");
    const auto LOGIN_BUF = Console_wrapper::get_inline_input<std::string>();
    if (ALL_ACCS.contains(LOGIN_BUF)) {
      Logger::Error("Аккаунт с таким логином уже существует");
      std::cin.get();
      return registration_form();
    }

    Console_wrapper::write("Введите пароль: ");
    const auto PASSW_BUF = Console_wrapper::get_inline_input<std::string>(true);

    Console_wrapper::write("Повторите пароль: ");
    const auto REP_PASSW_BUF = Console_wrapper::get_inline_input<std::string>(true);
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
    User::get_current_user() = std::make_unique<User>(LOGIN_BUF, PASSW_BUF, ROLE);
    return User::get_current_user();
  }
}  // namespace AUTHORIZATION_FORMS

[[nodiscard]] std::unique_ptr<User>& authorize() {
  auto&& force_reg = []() -> bool {
    if (User::get_json().empty()) return true;
    Console_wrapper::draw_frame(" Авторизация ");
    Console_wrapper::writeln("1) Cоздать новый аккаунт?");
    Console_wrapper::writeln("2) Войти в существующий");
    return Console_wrapper::get_inline_input<int16_t>() == 1;
  };
  return force_reg() ? AUTHORIZATION_FORMS::registration_form()
                     : AUTHORIZATION_FORMS::login_form();
}
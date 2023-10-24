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
[[nodiscard]] inline auto encrypt(T arg) {
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

   public:
    User(const nlohmann::json& json_obj) {}  // todo

    User(std::string_view login,
         std::string_view passw,
         User_role role) : user_role{role},
                           user_encrypted_passw{encrypt(passw)},
                           user_login{login} {
        to_global_json();
    }

    User(User&& u) : user_role{u.user_role},
                     user_id{u.user_id},
                     user_encrypted_passw{u.user_encrypted_passw},
                     user_login{u.user_login} {}

    ~User() = default;

    [[nodiscard]] auto is_admin() const { return user_role == User_role::admin; }
    [[nodiscard]] auto get_role() const { return; }
    [[nodiscard]] auto get_reader_ID() const { return std::format("{0:0>6}", user_id); }
    [[nodiscard]] auto get_passw() const { return; }
    [[nodiscard]] auto get_login() const { return; }

    inline void to_global_json() const {
        using namespace nlohmann;
        json& j = all_accs[user_login];
        j["Role"] = user_role;
        j["Password"] = user_encrypted_passw;
        j["ID"] = user_id;
    }
};

namespace {
    static User login() { return User("yenoo", "2354342", User_role::admin); }
    static User registration(){ return User("smth", "idc", User_role::user); }
}

User authorize() {
    return User::get_all_accs().empty() ? registration() : login();
}
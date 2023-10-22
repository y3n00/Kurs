#include <iostream>
#include <stacktrace>
#include <string>
#include <utility>

#include "thirdparty/json.hpp"

enum User_role {
    admin = 0,
    user
};

template <typename T>
inline auto encrypt(T passw) {
    return std::hash<T>{}(passw);
}

class User {
   private:
    static inline size_t global_user_id = 1;
    static inline nlohmann::json all_accs;

   public:
    static void load_accounts(std::string_view filename) {
        all_accs = std::move(load(filename));
    }
    static const auto& get_all_accs() { return all_accs; }

   private:
    User_role user_role{};
    size_t user_id{global_user_id++}, user_encrypted_passw{};
    std::string user_login{};

   public:
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

    bool is_admin() const { return user_role == User_role::admin; }

    auto get_role() const { return; }
    auto get_reader_ID() const { return std::format("{0:0>6}", user_id); }
    auto get_passw() const { return; }
    auto get_login() const { return; }

    inline void to_global_json() const {
        using namespace nlohmann;        
        json& j = all_accs[user_login];
        j["Role"] = user_role;
        j["Password"] = user_encrypted_passw;
        j["ID"] = user_id;
    }

    User(const nlohmann::json& json_obj) {
        std::cout << json_obj.dump(1);
        //  json_obj["Role"].get_to(user_role);
        //  json_obj["Login"].get_to(user_login);
        //  json_obj["Password"].get_to(user_encrypted_passw);
        //  json_obj["ID"].get_to(user_id);
        // ++global_user_id;
    }
};

User authorize() {
    const auto reg = []() -> User {

    };
    const auto log = []() -> User {

    };

    if (User::get_all_accs().empty()) {
        // return registration();
    }
    // return login();
    return User("y3n00", "12345678", User_role::admin);
}
#include <conio.h>

#include <memory>
#include <string_view>
constexpr std::string_view users_file = "users.json";
constexpr std::string_view books_file = "generated_books.json";

#include "../include/Book.hpp"
#include "../include/Console_wrapper.hpp"
#include "../include/Fsystem.hpp"
#include "../include/Library.hpp"
#include "../include/Log.hpp"
#include "../include/User.hpp"

// #define GENERATE
#ifdef GENERATE
#include "../include/Random.hpp"
#include "../include/Utils.hpp"
int main() {
    static Random r;
    constexpr size_t N = 200;
    for (size_t i = 0; i < N; i++) {
        auto&& title = r.generate_string(5, 30);
        auto&& in_lib = (bool)(r.get(0, 1));
        auto&& year = r.get(1900, 2023);
        auto&& pages = r.get(10, 500);
        auto&& author = r.generate_string(10, 35);
        auto&& publisher = r.generate_string(4, 15);
        Book(title, in_lib, year, pages, author, publisher);
    }
    FileSystem::save(books_file, Book::get_json());
    Logger::Success(N, "случайно сгенерированных книг было записано в", books_file);
}
#else

static void print_copyright() {
    std::println("Программа разработана БГАС");
    std::println("ИТ291 Клещинский Александр");
    std::println("Дата сборки: {}", __DATE__);
    system("pause");
    system("cls");
}

BOOL WINAPI on_exit_callback(DWORD reason) {
    if (reason == CTRL_CLOSE_EVENT) {
        FileSystem::save(users_file, User::get_json());
        FileSystem::save(books_file, Book::get_json());
        return true;
    }
}

int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    print_copyright();

    const std::string window_title = "lib App";
    Console::setFont(22, L"Consolas");
    Console::configure(window_title, {600, 400});
    Logger::new_line_enabled = false;

    Book::load_books(books_file);
    User::load_accounts(users_file);

    const std::unique_ptr<User>& USER = authorize();
    Console::setTitle(window_title + " | " + USER->get_login());
    SetConsoleCtrlHandler(on_exit_callback, true);

    const std::unique_ptr<ILibrary> lib = USER->is_admin() ? std::make_unique<Admin_lib>() : std::make_unique<User_lib>();
    static const auto& menu = lib->get_menu();
    do {
        auto&& selected = Console_wrapper::vec_pick<int16_t>(menu);
        Console_wrapper::draw_frame();
        lib->do_at(selected);
        Console_wrapper::new_line();
        Logger::Warning("Нажмите любую клавишу чтобы продолжить");
    } while (_getch() != Keys::ESCAPE);
}
#endif
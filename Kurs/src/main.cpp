#include <conio.h>

#include <memory>

#include "../include/Book.hpp"
#include "../include/Console_wrapper.hpp"
#include "../include/Fsystem.hpp"
#include "../include/Library.hpp"
#include "../include/Log.hpp"
#include "../include/User.hpp"

constexpr std::string_view accs_fname = "accs.json";
constexpr std::string_view books_fname = "books.json";

BOOL WINAPI on_exit_callback(DWORD reason) {
    if (reason == CTRL_CLOSE_EVENT) {
        FileSystem::save(accs_fname, User::get_all_accs());
        FileSystem::save(books_fname, Book::get_all_books());
        system("cls");
        return true;
    }
}

int main() {
    system("cls");              // clear screen
    SetConsoleCP(65001);        // encoding for ru lang
    SetConsoleOutputCP(65001);  //

    const std::string window_title = "Library App";  // titlebar buffer
    Console::setFont(18, L"Cascadia Mono");          // font setup
    Console::configure(window_title, {800, 600});    // setup console
    Console_wrapper::set_frame_border('|', '-');

    //////////////////////////////////////////////////////////////////////////////

    Book::load_books(books_fname);
    User::load_accounts(accs_fname);

    User::current_global_user = authorize();
    auto&& user = *User::current_global_user.get();
    Console::setTitle(window_title + " | " + user.get_login());
    SetConsoleCtrlHandler(on_exit_callback, true);  // only after successful login/reg

    const std::unique_ptr<ILibrary> LIBRARY = [is_admin = user.is_admin()] {
        return is_admin ? std::make_unique<Library_as_admin>()
                        : std::make_unique<Library_as_user>();
    }();

    const std::vector<std::string> MENU_ITEMS = LIBRARY->get_menu();
    static const uint16_t MIN_IDX = 0, MAX_IDX = MENU_ITEMS.size() - 1;
    auto&& MENU_CHOICE_CLAMP = std::bind(std::clamp<uint16_t>, std::placeholders::_1, MIN_IDX, MAX_IDX);
    do {
        Console_wrapper::draw_frame();
        Console_wrapper::vec_print(MENU_ITEMS);
        const auto choice = MENU_CHOICE_CLAMP(Console_wrapper::get_input<uint16_t>() - 1);
        Console_wrapper::clear_border();  // clear screen b4 doin smth
        LIBRARY->do_at(choice);
    } while (_getch() != Keys::ESCAPE);
}
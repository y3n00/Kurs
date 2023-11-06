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

int main() {
    SetConsoleCP(65001);        // set new encoding for ru lang
    SetConsoleOutputCP(65001);  //

    std::string windows_title = "Library App";      // dynamic titlebar buffer
    Console::Configure(windows_title, {960, 480});  // setup console with new title and size
    Console::setFont(18, L"Cascadia Mono");         // console font resize

    //////////////////////////////////////////////////////////////////////////////

    Book::load_books(books_fname);
    User::load_accounts(accs_fname);

    const auto user = authorize();
    Console::setTitle(windows_title + " | " + user.get_login());
    SetConsoleCtrlHandler(
        [](DWORD reason) -> BOOL {
            if (reason == CTRL_CLOSE_EVENT) {
                FileSystem::save(accs_fname, User::get_all_accs());
                FileSystem::save(books_fname, Book::get_all_books());
                system("cls");

                return true;
            }
        },
        true);  // on close callback to save data only after successful login/reg

    const auto library = [is_admin = user.is_admin()] {
        return is_admin
                   ? std::make_unique<Library_as_admin>()
                   : std::make_unique<Library_as_user>();
    }();

    const auto MENU_ITEMS = std::move(library->get_menu().str());
    static const size_t MIN_IDX = 1, MAX_IDX = library->get_menu_size();

    int choice = 0;
    do {
        Console_wrapper::draw_frame('|', '-');
        Console_wrapper::writeln(MENU_ITEMS);
        const auto choice_buf = Console_wrapper::get_input<int>();
        choice = std::clamp<int>(choice_buf, MIN_IDX, MAX_IDX);

        library->do_at(choice);
    } while (_getch() != Keys::ESCAPE);
}
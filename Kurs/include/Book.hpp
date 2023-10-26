#include <string>

#include "Fsystem.hpp"
#include "thirdparty/json.hpp"

class Book {
   private:
    static inline size_t global_book_id = 1;
    static inline nlohmann::json all_books{};

   public:
    static void load_books(std::string_view filename) {
        FileSystem::load(filename, all_books);
        if (all_books.empty()) {
            global_book_id = 1;
        } else {
            for (const auto& [_, data] : all_books.items())
                global_book_id = std::max(global_book_id, data.at("ID").get<size_t>());
            global_book_id += 1;
        }
    }

    [[nodiscard]] static const auto& get_all_books() { return all_books; }

   private:
    bool in_library{};
    uint16_t book_year{}, book_pages{};
    size_t book_id{global_book_id++}, last_reader{};
    std::string author_name{}, book_title{}, book_publisher{};

    inline void to_global_json() const {
        nlohmann::json& js = all_books[book_title];
        js["Author"] = author_name;
        js["Pages"] = book_pages;
        js["ID"] = book_id;
        js["Last reader"] = last_reader;
        js["Publisher"] = book_publisher;
        js["Year"] = book_year;
        js["In library"] = in_library;
    }

   public:
    Book(std::string_view title,
         const nlohmann::json& book_data) : author_name{book_data.at("Author").get<std::string>()},
                                            book_title{title},
                                            book_pages{book_data.at("Pages").get<uint16_t>()},
                                            book_id{book_data.at("ID").get<size_t>()},
                                            last_reader{book_data.at("Last reader").get<size_t>()},
                                            book_publisher{book_data.at("Publisher").get<std::string>()},
                                            book_year{book_data.at("Year").get<uint16_t>()},
                                            in_library{book_data.at("In library").get<bool>()} {}

    Book(Book&& other_book) noexcept : in_library{other_book.in_library},
                                       book_year{other_book.book_year},
                                       book_pages{other_book.book_pages},
                                       book_id{other_book.book_id},
                                       last_reader{other_book.last_reader},
                                       author_name{std::move(other_book.author_name)},
                                       book_title{std::move(other_book.book_title)},
                                       book_publisher{std::move(other_book.book_publisher)} {}

    Book(std::string_view title,
         bool in_lib,
         uint16_t year,
         uint16_t pages_num,
         std::string_view author,
         std::string_view publisher) : in_library{in_lib},
                                       book_year{year},
                                       book_pages{pages_num},
                                       author_name{author},
                                       book_title{title},
                                       book_publisher{publisher} {
        to_global_json();
    }
    ~Book() = default;

    [[nodiscard]] auto get_id() const { return book_id; }

    [[nodiscard]] auto is_in_library() const { return in_library; }
    void setin_library(bool new_value) { in_library = new_value; }

    [[nodiscard]] auto get_year() const { return book_year; }
    void set_year(uint16_t new_value) { book_year = new_value; }

    [[nodiscard]] auto get_pages() const { return book_pages; }
    void set_pages(uint16_t new_value) { book_pages = new_value; }

    [[nodiscard]] auto get_last_reader() const { return last_reader; }
    void set_last_reader(uint16_t new_value) { last_reader = new_value; }

    [[nodiscard]] auto get_author() const { return author_name; }
    void set_author(std::string_view new_value) { author_name = new_value; }

    [[nodiscard]] auto get_title() const { return book_title; }
    void set_title(std::string_view new_value) { book_title = new_value; }

    [[nodiscard]] auto get_publisher() const { return book_publisher; }
    void set_publisher(std::string_view new_value) { book_publisher = new_value; }
};
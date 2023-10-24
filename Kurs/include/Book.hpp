#include <string>

#include "fsystem.hpp"
#include "thirdparty/json.hpp"

class Book {
   private:
    static inline size_t global_book_id = 1;
    static inline nlohmann::json all_books{};

   public:
    static void load_books(std::string_view filename) {
        FileSystem::load(filename, all_books);
    }

    [[nodiscard]] static const auto& get_all_books() { return all_books; }

   private:
    bool in_library{};
    uint16_t book_year{}, book_pages{};
    size_t book_id{global_book_id++}, last_reader{};
    std::string author_name{}, book_title{}, book_publisher{};
    inline void save_to_global() const { all_books.push_back(this->to_json()); }

   public:
    Book(std::string_view title, bool in_lib,
         uint16_t year, uint16_t pages_num,
         std::string_view author,
         std::string_view publisher) : in_library{in_lib},
                                       book_year{year},
                                       book_pages{pages_num},
                                       author_name{author},
                                       book_title{title},
                                       book_publisher{publisher} {
        save_to_global();
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

    [[nodiscard]] nlohmann::json to_json() const {
        using namespace nlohmann;
        json js;
        js["Author"] = author_name;
        js["Title"] = book_title;
        js["Pages"] = book_pages;
        js["ID"] = book_id;
        js["Last reader"] = last_reader;
        js["Publisher"] = book_publisher;
        js["Year"] = book_year;
        js["In library"] = in_library;
        return js;
    }

    Book(const nlohmann::json& json_obj) {
        json_obj["Author"].get_to(author_name);
        json_obj["Title"].get_to(book_title);
        json_obj["Pages"].get_to(book_pages);
        json_obj["ID"].get_to(book_id);
        json_obj["Last reader"].get_to(last_reader);
        json_obj["Publisher"].get_to(book_publisher);
        json_obj["Year"].get_to(book_year);
        json_obj["In library"].get_to(in_library);
        ++global_book_id;
    }
};
#pragma once
#include <format>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include "Fsystem.hpp"
#include "thirdparty/json.hpp"

class Book {
 private:
  static inline size_t global_book_id = 1;
  static inline nlohmann::json books_json{};

 public:
  static void load_books(std::string_view filename) {
    FileSystem::load(filename, books_json);
    if (!books_json.empty()) {
      for (const auto& [_, data] : books_json.items())
        global_book_id =
            std::max(global_book_id, data.at("ID").get<size_t>());
      global_book_id += 1;
    }
  }
  [[nodiscard]] static const auto& get_json() { return books_json; }
  [[nodiscard]] static std::vector<Book> get_vector() {
    return books_json.items() |
           std::views::transform([](auto&& json_item) { return Book(json_item.key()); }) |
           std::ranges::to<std::vector<Book>>();
  }

 private:
  bool in_library{};
  uint16_t book_year{}, book_pages{};
  size_t book_id{global_book_id++}, last_reader{};
  std::string author_name, book_title, book_publisher;

 public:
  Book(std::string_view title) : book_title{title} {
    const auto& CURRENT_BOOK_DATA = books_json.at(book_title);
    CURRENT_BOOK_DATA["Author"].get_to(author_name);
    CURRENT_BOOK_DATA["Pages"].get_to(book_pages);
    CURRENT_BOOK_DATA["ID"].get_to(book_id);
    CURRENT_BOOK_DATA["Last reader"].get_to(last_reader);
    CURRENT_BOOK_DATA["Publisher"].get_to(book_publisher);
    CURRENT_BOOK_DATA["Year"].get_to(book_year);
    CURRENT_BOOK_DATA["In library"].get_to(in_library);
    update_data();
  }

  Book(std::string_view title, bool in_lib, uint16_t year,
       uint16_t pages_num, std::string_view author,
       std::string_view publisher)
      : in_library{in_lib},
        book_year{year},
        book_pages{pages_num},
        author_name{author},
        book_title{title},
        book_publisher{publisher} {
    update_data();
  }

  Book(Book&& other_book) noexcept
      : in_library{other_book.in_library},
        book_year{other_book.book_year},
        book_pages{other_book.book_pages},
        book_id{other_book.book_id},
        last_reader{other_book.last_reader},
        author_name{std::move(other_book.author_name)},
        book_title{std::move(other_book.book_title)},
        book_publisher{std::move(other_book.book_publisher)} {}

  ~Book() = default;

  [[nodiscard]] auto is_in_library() const { return in_library; }
  [[nodiscard]] auto get_year() const { return book_year; }
  [[nodiscard]] auto get_pages() const { return book_pages; }
  [[nodiscard]] auto get_id() const { return book_id; }
  [[nodiscard]] auto get_last_reader() const { return last_reader; }
  [[nodiscard]] auto get_author() const { return author_name; }
  [[nodiscard]] auto get_title() const { return book_title; }
  [[nodiscard]] auto get_publisher() const { return book_publisher; }
  [[nodiscard]] auto get_data() const {
    return std::vector<std::string>{
        std::format("Название: {}", book_title),
        std::format("Автор: {}", author_name),
        std::format("Издатель: {}", book_publisher),
        std::format("Год выпуска: {}", book_year),
        std::format("Кол-во страниц: {}", book_pages),
        std::format("Статус: {}", (in_library ? "в библиотеке" : "у читателя")),
    };
  }

  void toggle_status() { in_library = !in_library; }
  void set_year(uint16_t new_value) { book_year = new_value; }
  void set_pages(uint16_t new_value) { book_pages = new_value; }
  void set_last_reader(size_t new_value) { last_reader = new_value; }
  void set_author(std::string_view new_value) { author_name = new_value; }
  void set_title(std::string_view new_value) { book_title = new_value; }
  void set_publisher(std::string_view new_value) { book_publisher = new_value; }
  void update_data() const {
    nlohmann::json& js = books_json[book_title];
    js["Author"] = author_name;
    js["Pages"] = book_pages;
    js["ID"] = book_id;
    js["Last reader"] = last_reader;
    js["Publisher"] = book_publisher;
    js["Year"] = book_year;
    js["In library"] = in_library;
  }
};
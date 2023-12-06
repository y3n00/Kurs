#pragma once
#include <algorithm>
#include <format>
#include <string>
#include <vector>

#include "thirdparty/json.hpp"

// for same size in different languages
[[nodiscard]] constexpr size_t my_strlen(std::string_view str) {
    return std::ranges::count_if(str, [](char byte) { return ((byte & 0x80) == 0 || (byte & 0xC0) == 0xC0); });
}

[[nodiscard]] constexpr std::string cut_str(std::string_view str, size_t count) {
    const std::string as_str{str};
    return str.length() < count ? as_str : as_str.substr(0, count - 3) + "...";
}

[[nodiscard]] constexpr std::string remove_quotes(auto&& str) {
    return str.front() == '"' ? str.substr(1, str.length() - 2) : str;
}
// left alignment by default
[[nodiscard]] constexpr std::string place_by_width(auto&& str, uint16_t width, char pos = '<') {
    if (width <= str.size()) return str;
    return std::vformat(std::format("{{: {}{}}}", pos, width), std::make_format_args(str));
}

[[nodiscard]] constexpr std::string value_view(auto&& value) {
    return value.is_array() ? std::to_string(value.size()) : remove_quotes(nlohmann::to_string(value));
}

[[nodiscard]] inline std::vector<nlohmann::json> represent_json(const nlohmann::json& json_obj) {
    std::vector<nlohmann::json> json_objects;
    for (auto&& [key, sub_json] : json_obj.items()) {
        nlohmann::json new_obj = sub_json;
        new_obj["Title"] = key;
        json_objects.emplace_back(new_obj);
    }
    return json_objects;
}

[[nodiscard]] size_t encrypt_str(std::string_view arg, size_t key) {  // Caesar + hash
    return std::hash<std::string>{}(arg |
                                    std::views::transform([key](char ch) { return ch + key; }) |
                                    std::ranges::to<std::string>());
}
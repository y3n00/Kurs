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

[[nodiscard]] constexpr std::string place_by_width(const std::string& str, uint16_t width, char pos = '<') {
    if (width <= str.size()) return str;
    return std::vformat(std::format("{{: {}{}}}", pos, width), std::make_format_args(str));
}

[[nodiscard]] constexpr std::string value_view(auto&& value) {
    return value.is_array() ? std::to_string(value.size())
                            : remove_quotes(nlohmann::to_string(value));
}

[[nodiscard]] inline std::vector<nlohmann::json> represent_json(const nlohmann::json& js_obj) {
    std::vector<nlohmann::json> REPRESENTED;
    for (auto&& [k, v] : js_obj.items()) {
        nlohmann::json j = v;
        j["Title"] = k;
        REPRESENTED.emplace_back(j);
    }
    return REPRESENTED;
}
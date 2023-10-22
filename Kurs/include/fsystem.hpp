#include <filesystem>
#include <fstream>
#include <string_view>

#include "thirdparty/json.hpp"

using TYPE = nlohmann::json;

inline TYPE load(std::string_view filename) {
    TYPE temp{};
    if (!std::filesystem::exists(filename.data()))
        return temp;

    std::ifstream(filename.data()) >> temp;
    return temp;
}

inline void save(std::string_view filename, const TYPE& data) {
    if (!data.empty())
        std::ofstream(filename.data(), std::ofstream::trunc) << data;
}
#pragma once
#include <filesystem>
#include <fstream>
#include <string>

namespace FileSystem {
    inline void load(std::string_view filename, auto& store_to) {
        if (std::filesystem::exists(filename.data()))
            std::ifstream(filename.data()) >> store_to;
    }

    inline void save(std::string_view filename, const auto& data) {
        if (data != decltype(data){})  // object isn't empty
            std::ofstream(filename.data(), std::ofstream::trunc) << data;
    }
}  // namespace FileSystem
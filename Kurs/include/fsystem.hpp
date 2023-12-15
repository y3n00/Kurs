#pragma once
#include <filesystem>
#include <fstream>
#include <string>

namespace FileSystem {
  inline void load(std::string_view fname, auto& store_to) {
    if (std::filesystem::exists(fname.data()))
      std::ifstream(fname.data()) >> store_to;
  }

  inline void save(std::string_view fname, const auto& data) {
    if (data != decltype(data){})
      std::ofstream(fname.data(), std::ofstream::trunc) << data;
  }
}  // namespace FileSystem
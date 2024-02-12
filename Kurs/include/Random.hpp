#pragma once
#include <limits>
#include <random>
#include <string>

template <std::integral Ty = uint32_t>
class Random {
    using typeLimit = std::numeric_limits<Ty>;

   private:
    std::mt19937 gen{std::random_device{}()};

   public:
    Random() = default;
    Random(Random&&) = delete;
    Random(const Random&) = delete;
    Random& operator=(const Random&) = delete;

    [[nodiscard]] inline auto get(Ty min = typeLimit::min(), Ty max = typeLimit::max()) {
        return std::uniform_int_distribution(min, max)(gen);
    }

    [[nodiscard]] auto generate_string(uint8_t min_len = 1, uint8_t max_len = std::numeric_limits<uint8_t>::max) {
        constexpr static std::string_view SYMBOLS =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "1234567890";
        const auto strLen = get(min_len, max_len);
        std::string result(strLen, ' ');
        for (char& ch : result)
            ch = SYMBOLS[get(strLen - 1)];
        return result;
    }
};
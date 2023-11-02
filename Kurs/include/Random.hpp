#pragma once

#include <random>
template <typename T = uint32_t>
class Random {
   private:
    std::mt19937 gen{std::random_device{}()};

   public:
    Random() = default;
    Random(Random&&) = delete;
    Random(const Random&) = delete;
    Random& operator=(const Random&) = delete;

    [[nodiscard]] T get() {
        return std::uniform_int_distribution()(gen);
    }

    [[nodiscard]] T get(T maxN) {
        return std::uniform_int_distribution(T{}, maxN)(gen);
    }

    [[nodiscard]] T get(T minN, T maxN) {
        return std::uniform_int_distribution(minN, maxN)(gen);
    }

    [[nodiscard]] auto generate_string(uint8_t min_len = 1, uint8_t max_len = std::numeric_limits<uint8_t>::max) {
        constexpr static std::string_view SYMBOLS =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
            "1234567890"
            "!@#$%^&*(){}[]_+=-?/.,";
        std::string result(get(min_len, max_len), ' ');
        for (char& ch : result)
            ch = SYMBOLS[get(SYMBOLS.length() - 1)];
        return result;
    }
};
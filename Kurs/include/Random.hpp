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
};
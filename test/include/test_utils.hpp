#pragma once
#include <x86intrin.h>
#include <cstdint>
#include <random>
#include <string>

namespace test {

struct TestObj {
    std::string uid;
    uint32_t seq;
};

/* read TSC timestamp. */
inline uint64_t rdtscp() {
    uint32_t _;
    return __rdtscp(&_);
}

/* generate a random number of type T in range [beg, end]. */
template <typename T>
inline T random(T beg, T end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<T> dis(beg, end);
    return dis(gen);
}

}  // namespace test
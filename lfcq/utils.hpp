#pragma once
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <functional>
#ifndef NDEBUG
#include <filesystem>
#include <fstream>
#include <iostream>
#endif

namespace lfcq {

#define QUEUE_MAX_SIZE 0x8000'0000U

/* align the input UP to power of 2. */
inline uint32_t alignUpPowOf2(uint32_t val) {
    if (val == 0) return 1;

    // in case overflow or val is already power of 2
    val = std::min(val, QUEUE_MAX_SIZE) - 1;
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;

    return val + 1;
}

#undef QUEUE_MAX_SIZE

/* automatically generate <push> function for derivative types of T. */
template <typename U, typename T>
concept RelatedTo = std::same_as<U, T> || std::same_as<U, const T> || std::same_as<U, T&> || std::same_as<U, const T&>;

/* callback when the user wishes to manually assign to some members of the object */
template <typename T>
using PushHandle = std::function<void(T&)>;

/* callback when there is an element popped from the queue. */
template <typename T>
using PopHandle = std::function<void(T&)>;

/* cache-friendly wrapper for user's data structure. */
template <typename T>
struct alignas(64) Aligned {
    T data;
    static_assert(alignof(Aligned<T>) == 64);
};

#ifndef NDEBUG
/* an encapsulation for easier management on file stream. */
class Dumper {
  private:
    std::ofstream file_;

  public:
    explicit Dumper(const std::string&& path) : file_(std::filesystem::path(path)) {
        if (!file_) {
            std::cout << "[Dumper] fail to open or create dump file: " << path << std::endl;
            exit(1);
        }
    }

    ~Dumper() { file_.close(); }

    template <typename... Args>
    void dump(Args&&... args) {
        (file_ << ... << std::forward<Args>(args)) << std::endl;
    }
};
#endif

}  // namespace lfcq

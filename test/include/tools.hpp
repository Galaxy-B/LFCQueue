#pragma once
#include <x86intrin.h>
#include <cstdint>
#include <memory>
#include <random>

namespace test {

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

/* customized memory allocator for test */
template <typename T>
class TestAllocator {
  public:
    // record the size of (de)allocation for comparison in test
    std::shared_ptr<uint32_t> alloc_n = std::make_shared<uint32_t>(0);
    std::shared_ptr<uint32_t> dealloc_n = std::make_shared<uint32_t>(0);

    // we use pointer to map changes on the queue's inner allocator to the outer one
    TestAllocator() = default;
    TestAllocator(const TestAllocator& other) : alloc_n(other.alloc_n), dealloc_n(other.dealloc_n) {}

    T* allocate(size_t n) {
        *alloc_n += n;
        return static_cast<T*>(operator new(sizeof(T) * n));
    }

    void deallocate(T* ptr, size_t n) {
        *dealloc_n += n;
        operator delete(ptr);
    }
};

}  // namespace test
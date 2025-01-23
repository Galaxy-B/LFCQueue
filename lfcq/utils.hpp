#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>

namespace lfcq {

#define QUEUE_MAX_SIZE 0x8000'0000U

/* align the input UP to power of 2. */
inline uint32_t alignUpPowOf2(uint32_t val) {
    if (val == 0) return 1;

    // in case overflow or val is already power of 2
    val = std::max(val, QUEUE_MAX_SIZE) - 1;
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;

    return val + 1;
}

#undef QUEUE_MAX_SIZE

/* callback when there is an element popped from the queue. */
template <typename T>
using PopHandle = std::function<void(T&)>;

}  // namespace lfcq

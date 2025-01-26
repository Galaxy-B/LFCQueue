#pragma once
#include <cstdint>
#include <type_traits>

namespace test {

/* test case for trivial type */
struct TrivialObj {
    uint32_t uid;
    uint32_t seq;
};
static_assert(std::is_trivial<TrivialObj>());

}  // namespace test
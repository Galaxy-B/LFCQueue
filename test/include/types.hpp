#pragma once
#include <cstdint>
#include <ostream>
#include <type_traits>

namespace test {

/* test case for trivial type */
struct TrivialObj {
    uint32_t uid;
    uint32_t seq;

    friend std::ostream& operator<<(std::ostream& os, const TrivialObj& obj) {
        return os << obj.uid << ", " << obj.seq;
    }
};
static_assert(std::is_trivial<TrivialObj>());

}  // namespace test
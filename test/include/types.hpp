#pragma once
#include <cstdint>
#include <type_traits>

namespace test {

/* test case for trivial type */
struct TrivialObj {
    uint32_t uid;
    uint32_t seq;

    bool operator==(const TrivialObj& other) const { return uid == other.uid && seq == other.seq; }
};
static_assert(std::is_trivial<TrivialObj>());

/* test case for non-trivial type */
struct NonTrivialObj {
    uint32_t uid;
    uint32_t seq;

    NonTrivialObj(uint32_t _uid, uint32_t _seq) : uid(_uid), seq(_seq) {}

    bool operator==(const NonTrivialObj& other) const { return uid == other.uid && seq == other.seq; }
};
static_assert(!std::is_trivial<NonTrivialObj>());

}  // namespace test
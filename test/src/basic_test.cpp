#include <gtest/gtest.h>

#include "basic_queue.hpp"
#include "tools.hpp"
#include "types.hpp"
#include "utils.hpp"

using namespace lfcq;
using namespace test;

template <typename T>
class BasicTest : public testing::Test {
  protected:
    TestAllocator<T> allocator_;
    uint32_t size_;

    BasicTest() : size_(1000) {}
};

using TestTypes = testing::Types<TrivialObj, NonTrivialObj>;
TYPED_TEST_SUITE(BasicTest, TestTypes);

TYPED_TEST(BasicTest, AllocatorTest) {
    using Allocator = TestAllocator<TypeParam>;

    // create a queue with lifetime limited in the following block
    {
        BasicQueue<TypeParam, Allocator> queue(this->size_, this->allocator_);
        EXPECT_EQ(*(this->allocator_.alloc_n), alignUpPowOf2(this->size_));
    }

    // the queue should have been deconstructed
    EXPECT_EQ(*(this->allocator_.dealloc_n), alignUpPowOf2(this->size_));
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    testing::GTEST_FLAG(color) = "yes";
    return RUN_ALL_TESTS();
}

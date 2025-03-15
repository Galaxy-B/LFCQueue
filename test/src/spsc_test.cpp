#include <gtest/gtest.h>

#include "spsc_queue.hpp"
#include "tools.hpp"
#include "types.hpp"

using namespace lfcq;
using namespace test;

template <typename T>
class SpscTest : public testing::Test {
  protected:
    SpscQueue<T> queue_;
    uint32_t cnt_;
    uint32_t uid_;

    // what we write to / read from the queue will be pushed to these two vectors respectively
    std::vector<T> writer_;
    std::vector<T> reader_;

    // by far we only have one interface for reader
    const std::function<void()> pop = [this]() {
        for (uint32_t i = 0; i < this->cnt_;) {
            this->queue_.pop([this, &i](T& obj) {
                this->reader_.emplace_back(obj);
                i++;
            });
        }
    };

    SpscTest() : queue_(2000), cnt_(2000), uid_(random(0U, UINT32_MAX)) {}
};

using TestTypes = testing::Types<TrivialObj, NonTrivialObj>;
TYPED_TEST_SUITE(SpscTest, TestTypes);

TYPED_TEST(SpscTest, PushInterfaceTest) {
    std::thread writer([this]() {
        for (uint32_t i = 0; i < this->cnt_; i++) {
            TypeParam obj(this->uid_, i);
            this->queue_.push(obj);
            this->writer_.emplace_back(obj);
        }
    });

    std::thread reader(this->pop);

    writer.join();
    reader.join();
    EXPECT_EQ(this->writer_, this->reader_);
}

TYPED_TEST(SpscTest, ManualPushInterfaceTest) {
    std::thread writer([this]() {
        for (uint32_t i = 0; i < this->cnt_; i++) {
            this->queue_.push([this, i](TypeParam& dst) {
                memcpy(&dst.uid, &this->uid_, sizeof(uint32_t));
                memcpy(&dst.seq, &i, sizeof(uint32_t));
            });
            this->writer_.emplace_back(this->uid_, i);
        }
    });

    std::thread reader(this->pop);

    writer.join();
    reader.join();
    EXPECT_EQ(this->writer_, this->reader_);
}

TYPED_TEST(SpscTest, EmplaceInterfaceTest) {
    std::thread writer([this]() {
        for (uint32_t i = 0; i < this->cnt_; i++) {
            this->queue_.emplace(this->uid_, i);
            this->writer_.emplace_back(this->uid_, i);
        }
    });

    std::thread reader(this->pop);

    writer.join();
    reader.join();
    EXPECT_EQ(this->writer_, this->reader_);
}

TYPED_TEST(SpscTest, LoopWriteTest) {
    // there will be slots written to more than one times, a.k.a loop write
    this->cnt_ = 4000;

    std::thread writer([this]() {
        for (uint32_t i = 0; i < this->cnt_; i++) {
            this->queue_.emplace(this->uid_, i);
            this->writer_.emplace_back(this->uid_, i);
            // in case of emplace failure because there is no time for reader to consume
            usleep(i % 1000 == 999 ? 100'000 : 0);
        }
    });

    std::thread reader(this->pop);

    writer.join();
    reader.join();
    EXPECT_EQ(this->writer_, this->reader_);
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    testing::GTEST_FLAG(color) = "yes";
    return RUN_ALL_TESTS();
}
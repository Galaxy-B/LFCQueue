#include <gtest/gtest.h>

#include "mpmc_unique_queue.hpp"
#include "tools.hpp"
#include "types.hpp"

using namespace lfcq;
using namespace test;

template <typename T>
class MpmcUniqueTest : public testing::Test {
  protected:
    // how many producers / consumers we wish to have simultaneously
    static constexpr uint32_t multiple_cnt = 3;

    MpmcUniqueQueue<T> queue_;
    uint32_t cnt_;
    uint32_t uid_;

    // there will be multiple producers / consumers sharing a same counter
    std::atomic<uint32_t> w_cnt_;
    std::atomic<uint32_t> r_cnt_;

    // use checksum to easily test write / read consistency while enabling concurrency
    std::atomic<uint32_t> w_checksum_;
    std::atomic<uint32_t> r_checksum_;

    // in all test cases for MPMC queues we apply fixed interface combination
    const std::function<void()> push = [this]() {
        // <fetch_add> is necessary to make sure that exact <cnt_> of elements got pushed to queue
        while (this->w_cnt_.fetch_add(1) < this->cnt_) {
            uint32_t seq = random(0U, UINT32_MAX);
            this->queue_.emplace(this->uid_, seq);
            this->w_checksum_ ^= seq;
        }
    };

    const std::function<void()> pop = [this]() {
        for (uint32_t seq = 0; this->r_cnt_ < this->cnt_;) {
            // we only check uid and fetch seq inside the handle so as to simulate real concurrency
            this->queue_.pop([this, &seq](T& obj) {
                EXPECT_EQ(obj.uid, this->uid_);
                seq = obj.seq;
            });

            // notice that checksum won't change when seq = 0
            this->r_checksum_ ^= seq;
            this->r_cnt_ += (std::exchange(seq, 0) != 0);
        }
    };

    MpmcUniqueTest() : queue_(4000), cnt_(4000), uid_(random(0U, UINT32_MAX)) {}
};

using TestTypes = testing::Types<TrivialObj, NonTrivialObj>;
TYPED_TEST_SUITE(MpmcUniqueTest, TestTypes);

// multiple producers & single consumer
TYPED_TEST(MpmcUniqueTest, MpscTest) {
    std::vector<std::thread> writers;
    for (uint32_t i = 0; i < this->multiple_cnt; i++) {
        writers.emplace_back(this->push);
    }

    // current thread as reader thread
    this->pop();

    for (auto& writer : writers) {
        writer.join();
    }
    EXPECT_EQ(this->w_checksum_, this->r_checksum_);
}

// single producer & multiple consumers
TYPED_TEST(MpmcUniqueTest, SpmcTest) {
    std::vector<std::thread> readers;
    for (uint32_t i = 0; i < this->multiple_cnt; i++) {
        readers.emplace_back(this->pop);
    }

    // current thread as writer thread
    this->push();

    for (auto& reader : readers) {
        reader.join();
    }
    EXPECT_EQ(this->w_checksum_, this->r_checksum_);
}

// multiple producers & multiple consumers
TYPED_TEST(MpmcUniqueTest, MpmcTest) {
    std::vector<std::thread> writers;
    for (uint32_t i = 0; i < this->multiple_cnt; i++) {
        writers.emplace_back(this->push);
    }

    std::vector<std::thread> readers;
    for (uint32_t i = 0; i < this->multiple_cnt; i++) {
        readers.emplace_back(this->pop);
    }

    for (uint32_t i = 0; i < this->multiple_cnt; i++) {
        writers.at(i).join();
        readers.at(i).join();
    }
    EXPECT_EQ(this->w_checksum_, this->r_checksum_);
}

TYPED_TEST(MpmcUniqueTest, LoopWriteTest) {
    // there will be slots written to more than one times, a.k.a loop write
    this->cnt_ = 8000;

    // use SPMC scenario so writer can definitely give away CPU under our control
    std::vector<std::thread> readers;
    for (uint32_t i = 0; i < this->multiple_cnt; i++) {
        readers.emplace_back(this->pop);
    }

    // current thread as writer thread
    for (uint32_t i = 0; i < this->cnt_; i++) {
        uint32_t seq = random(0U, UINT32_MAX);
        this->queue_.emplace(this->uid_, seq);
        this->w_checksum_ ^= seq;

        // periodically give away CPU
        usleep(i % 1000 == 999 ? 100'000 : 0);
    }

    for (auto& reader : readers) {
        reader.join();
    }
    EXPECT_EQ(this->w_checksum_, this->r_checksum_);
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    testing::GTEST_FLAG(color) = "yes";
    return RUN_ALL_TESTS();
}

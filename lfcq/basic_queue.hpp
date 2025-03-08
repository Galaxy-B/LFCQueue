#pragma once
#include <cstdint>
#include "utils.hpp"

namespace lfcq {

/* base class for all types of lock-free circular queues. */
/* NOTE: available for moving but not for copying. */
/* NOTE: all callbacks provided by user are forbidden to throw exception. */
/* NOTE: user can customize the memory allocator for the queue. */
template <typename T, typename Allocator>
class BasicQueue {
  protected:
    Allocator alloc_;
    uint32_t size_;
    uint32_t mask_;
    T* queue_;

  public:
    BasicQueue(uint32_t size, const Allocator& alloc) : alloc_(alloc) {
        size_ = alignUpPowOf2(size);
        mask_ = size_ - 1;
        queue_ = alloc_.allocate(size_);
    }

    virtual ~BasicQueue() {
        if (queue_) {
            alloc_.deallocate(queue_, size_);
        }
    }

    BasicQueue(const BasicQueue& other) = delete;
    BasicQueue& operator=(const BasicQueue& other) = delete;

    BasicQueue(BasicQueue&& other) noexcept : alloc_(std::move(other.alloc_)) {
        size_ = other.size_;
        mask_ = other.mask_;

        queue_ = other.queue_;
        other.queue_ = nullptr;
    }

    BasicQueue& operator=(BasicQueue&& other) noexcept {
        if (this != other) {
            size_ = other.size_;
            mask_ = other.mask_;

            alloc_ = std::move(other.alloc_);
            queue_ = other.queue_;
            other.queue_ = nullptr;
        }
        return *this;
    }

#ifndef NDEBUG
    /* dump the content of the queue on range [beg, end). */
    void dump(uint32_t beg, uint32_t end, std::string&& path) {
        Dumper dumper(std::move(path));
        for (uint32_t i = beg; i != end; i++) {
            dumper.dump(i & mask_, ": {", queue_[i & mask_], "}");
        }
    }
#endif
};

}  // namespace lfcq
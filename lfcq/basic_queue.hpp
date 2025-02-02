#pragma once
#include <cstdint>
#include "utils.hpp"

namespace lfcq {

/* base class for all types of lock-free circular queues. */
/* NOTE: available for moving but not for copying. */
template <typename T>
class BasicQueue {
  protected:
    uint32_t size_;
    uint32_t mask_;
    T* queue_;

  public:
    explicit BasicQueue(uint32_t size) {
        size_ = alignUpPowOf2(size);
        mask_ = size_ - 1;
        queue_ = reinterpret_cast<T*>(new char[sizeof(T) * size_]);
    }

    virtual ~BasicQueue() {
        if (queue_) {
            delete[] reinterpret_cast<char*>(queue_);
        }
    }

    BasicQueue(const BasicQueue& other) = delete;
    BasicQueue& operator=(const BasicQueue& other) = delete;

    BasicQueue(BasicQueue&& other) {
        size_ = other.size_;
        mask_ = other.mask_;

        queue_ = other.queue_;
        other.queue_ = nullptr;
    }

    BasicQueue& operator=(BasicQueue&& other) {
        if (this != other) {
            size_ = other.size_;
            mask_ = other.mask_;

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
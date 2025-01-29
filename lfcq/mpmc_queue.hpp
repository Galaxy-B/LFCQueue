#pragma once
#include <atomic>
#include <cstdint>
#include "utils.hpp"

namespace lfcq {

/* multiple producer multiple consumer lock-free circular queue. */
/* NOTE: available for moving but not for copying. */
template <typename T>
class MpmcQueue {
  private:
    std::atomic<uint32_t> next_w_;
    std::atomic<uint32_t> done_w_;
    std::atomic<uint32_t> next_r_;
    std::atomic<uint32_t> done_r_;
    uint32_t size_;
    uint32_t mask_;
    T* queue_;

  public:
    explicit MpmcQueue(uint32_t size) {
        size_ = alignUpPowOf2(size);
        mask_ = size_ - 1;
        queue_ = reinterpret_cast<T*>(new char[sizeof(T) * size_]);
    }

    ~MpmcQueue() {
        if (queue_) {
            delete[] reinterpret_cast<char*>(queue_);
        }
    }

    MpmcQueue(const MpmcQueue& other) = delete;
    MpmcQueue& operator=(const MpmcQueue& other) = delete;

    MpmcQueue(MpmcQueue&& other) {
        next_w_ = other.next_w_;
        done_w_ = other.done_w_;
        next_r_ = other.next_r_;
        done_r_ = other.done_r_;

        size_ = other.size_;
        mask_ = other.mask_;

        queue_ = other.queue_;
        other.queue_ = nullptr;
    }

    MpmcQueue& operator=(MpmcQueue&& other) {
        if (this != other) {
            next_w_ = other.next_w_;
            done_w_ = other.done_w_;
            next_r_ = other.next_r_;
            done_r_ = other.done_r_;

            size_ = other.size_;
            mask_ = other.mask_;

            queue_ = other.queue_;
            other.queue_ = nullptr;
        }
    }

    /* push an object to the end of the queue. */
    /* return false if the queue is full now, otherwise true. */
    template <typename U>
    bool push(U&& obj) requires RelatedTo<U, T> {
        // try to acquire a place for the current push
        uint32_t idx_w = next_w_.load(std::memory_order_acquire);
        do {
            if (idx_w - done_r_ == size_) return false;
        } while (!next_w_.compare_exchange_weak(idx_w, idx_w + 1));

        queue_[idx_w & mask_] = std::forward<T>(obj);

        // mark the current push has done after writing
        while (done_w_ != idx_w) {}
        done_w_.fetch_add(1, std::memory_order_acq_rel);
        return true;
    }

    /* directly construct an object at the end of the queue. */
    /* return false if the queue is full now, otherwise true. */
    template <typename... Args>
    bool emplace(Args&&... args) {
        // try to acquire a place for the current emplacement
        uint32_t idx_w = next_w_.load(std::memory_order_acquire);
        do {
            if (idx_w - done_r_ == size_) return false;
        } while (!next_w_.compare_exchange_weak(idx_w, idx_w + 1));

        new (&queue_[idx_w & mask_]) T(std::forward<Args>(args)...);

        // mark the current emplacement has done after writing
        while (done_w_ != idx_w) {}
        done_w_.fetch_add(1, std::memory_order_acq_rel);
        return true;
    }

    /* pop an object from the front of the queue, and handle it with the callback user provides. */
    /* return false if the queue is empty now, otherwise true. */
    bool pop(PopHandle<T>&& handle) {
        // try to lock down a index and pop element from it
        uint32_t idx_r = next_r_.load(std::memory_order_acquire);
        do {
            if (idx_r == done_w_) return false;
        } while (!next_r_.compare_exchange_weak(idx_r, idx_r + 1));

        handle(queue_[idx_r & mask_]);

        // mark the current pop has done after handling the element
        while (done_r_ != idx_r) {}
        done_r_.fetch_add(1, std::memory_order_acq_rel);
        return true;
    }
};

}  // namespace lfcq
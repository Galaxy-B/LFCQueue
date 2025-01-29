#pragma once
#include <atomic>
#include <cstdint>
#include <utility>
#include "utils.hpp"

namespace lfcq {

/* single producer single consumer lock-free circular queue. */
/* NOTE: available for moving but not for copying. */
template <typename T>
class SpscQueue {
  private:
    std::atomic<uint32_t> head_;
    std::atomic<uint32_t> tail_;
    uint32_t size_;
    uint32_t mask_;
    T* queue_;

  public:
    explicit SpscQueue(uint32_t size) {
        size_ = alignUpPowOf2(size);
        mask_ = size_ - 1;
        queue_ = reinterpret_cast<T*>(new char[sizeof(T) * size_]);
    }

    ~SpscQueue() {
        if (queue_) {
            delete[] reinterpret_cast<char*>(queue_);
        }
    }

    SpscQueue(const SpscQueue& other) = delete;
    SpscQueue& operator=(const SpscQueue& other) = delete;

    SpscQueue(SpscQueue&& other) {
        head_ = other.tail_;
        tail_ = other.tail_;
        size_ = other.size_;
        mask_ = other.mask_;

        queue_ = other.queue_;
        other.queue_ = nullptr;
    }

    SpscQueue& operator=(SpscQueue&& other) {
        if (this != &other) {
            head_ = other.tail_;
            tail_ = other.tail_;
            size_ = other.size_;
            mask_ = other.mask_;

            queue_ = other.queue_;
            other.queue_ = nullptr;
        }
        return *this;
    }

    /* push an object to the end of the queue. */
    /* return false if the queue is full now, otherwise true. */
    template <typename U>
    bool push(U&& obj) requires RelatedTo<U, T> {
        uint32_t head = head_.load(std::memory_order_acquire);
        if (tail_ - head == size_) return false;

        queue_[tail_ & mask_] = std::forward<T>(obj);

        tail_.fetch_add(1, std::memory_order_acq_rel);
        return true;
    }

    /* directly construct an object at the end of the queue. */
    /* return false if the queue is full now, otherwise true. */
    template <typename... Args>
    bool emplace(Args&&... args) {
        uint32_t head = head_.load(std::memory_order_acquire);
        if (tail_ - head == size_) return false;

        new (&queue_[tail_ & mask_]) T(std::forward<Args>(args)...);

        tail_.fetch_add(1, std::memory_order_acq_rel);
        return true;
    }

    /* pop an object from the front of the queue, and handle it with the callback user provides. */
    /* return false if the queue is empty now, otherwise true. */
    bool pop(PopHandle<T>&& handle) {
        uint32_t tail = tail_.load(std::memory_order_acquire);
        if (head_ == tail) return false;

        handle(queue_[head_ & mask_]);

        head_.fetch_add(1, std::memory_order_acq_rel);
        return true;
    }
};

}  // namespace lfcq

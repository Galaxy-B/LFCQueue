#pragma once
#include <atomic>
#include "basic_queue.hpp"
#include "utils.hpp"

namespace lfcq {

/* single producer single consumer lock-free circular queue. */
/* NOTE: available for moving but not for copying. */
template <typename T>
class SpscQueue : public BasicQueue<T> {
  private:
    std::atomic<uint32_t> head_;
    std::atomic<uint32_t> tail_;

  public:
    explicit SpscQueue(uint32_t size) : BasicQueue<T>(size) {}

    SpscQueue(const SpscQueue& other) = delete;
    SpscQueue& operator=(const SpscQueue& other) = delete;

    SpscQueue(SpscQueue&& other) : BasicQueue<T>(std::move(other)) {
        head_ = other.head_;
        tail_ = other.tail_;
    }

    SpscQueue& operator=(SpscQueue&& other) {
        if (this != &other) {
            head_ = other.head_;
            tail_ = other.tail_;

            BasicQueue<T>::operator=(std::move(other));
        }
        return *this;
    }

    /* push an object to the end of the queue. */
    /* return false if the queue is full now, otherwise true. */
    template <typename U>
    bool push(U&& obj) requires RelatedTo<U, T> {
        uint32_t head = head_.load(std::memory_order_acquire);
        if (tail_ - head == this->size_) return false;

        this->queue_[tail_ & this->mask_] = std::forward<T>(obj);

        tail_.fetch_add(1, std::memory_order_acq_rel);
        return true;
    }

    /* directly construct an object at the end of the queue. */
    /* return false if the queue is full now, otherwise true. */
    template <typename... Args>
    bool emplace(Args&&... args) {
        uint32_t head = head_.load(std::memory_order_acquire);
        if (tail_ - head == this->size_) return false;

        new (&this->queue_[tail_ & this->mask_]) T(std::forward<Args>(args)...);

        tail_.fetch_add(1, std::memory_order_acq_rel);
        return true;
    }

    /* pop an object from the front of the queue, and handle it with the callback user provides. */
    /* return false if the queue is empty now, otherwise true. */
    bool pop(PopHandle<T>&& handle) {
        uint32_t tail = tail_.load(std::memory_order_acquire);
        if (head_ == tail) return false;

        handle(this->queue_[head_ & this->mask_]);

        head_.fetch_add(1, std::memory_order_acq_rel);
        return true;
    }

#ifndef NDEBUG
    /* enabled in debug mode to dump the content of the queue.*/
    void dump(std::string&& path) { BasicQueue<T>::dump(head_, tail_, std::move(path)); }
#endif
};

}  // namespace lfcq

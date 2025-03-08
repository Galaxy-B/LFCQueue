#pragma once
#include <atomic>
#include "basic_queue.hpp"
#include "utils.hpp"

namespace lfcq {

/* multiple producer multiple consumer lock-free circular queue. */
/* MULTIPLE consumers allowed to manipulate a certain element simultaneously. */
/* NOTE: available for moving but not for copying. */
template <typename T, typename Allocator = std::allocator<T>>
class MpmcSharedQueue : public BasicQueue<T, Allocator> {
  private:
    std::atomic<uint32_t> next_w_;
    std::atomic<uint32_t> done_w_;
    std::atomic<uint32_t> done_r_;

  public:
    MpmcSharedQueue(uint32_t size, const Allocator& alloc = Allocator()) : BasicQueue<T, Allocator>(size, alloc) {}

    MpmcSharedQueue(const MpmcSharedQueue& other) = delete;
    MpmcSharedQueue& operator=(const MpmcSharedQueue& other) = delete;

    MpmcSharedQueue(MpmcSharedQueue&& other) noexcept : BasicQueue<T, Allocator>(std::move(other)) {
        next_w_ = other.next_w_;
        done_w_ = other.done_w_;
        done_r_ = other.done_r_;
    }

    MpmcSharedQueue& operator=(MpmcSharedQueue&& other) noexcept {
        if (this != other) {
            next_w_ = other.next_w_;
            done_w_ = other.done_w_;
            done_r_ = other.done_r_;

            BasicQueue<T, Allocator>::operator=(std::move(other));
        }
        return *this;
    }

    /* push an object to the end of the queue. */
    /* return false if the queue is full now, otherwise true. */
    template <typename U>
    bool push(U&& obj) noexcept requires RelatedTo<U, T> {
        // try to acquire a place for the current push
        uint32_t idx_w = next_w_.load(std::memory_order_acquire);
        do {
            if (idx_w - done_r_ == this->size_) return false;
        } while (!next_w_.compare_exchange_weak(idx_w, idx_w + 1));

        this->queue_[idx_w & this->mask_] = std::forward<T>(obj);

        // mark the current push has done after writing
        while (done_w_ != idx_w) {}
        done_w_.fetch_add(1, std::memory_order_acq_rel);
        return true;
    }

    /* call this push interface when you wish to manually initialize the object */
    /* return false if the queue is full now, otherwise true. */
    bool push(PushHandle<T>&& handle) noexcept {
        // try to acquire a place for the current push
        uint32_t idx_w = next_w_.load(std::memory_order_acquire);
        do {
            if (idx_w - done_r_ == this->size_) return false;
        } while (!next_w_.compare_exchange_weak(idx_w, idx_w + 1));

        handle(this->queue_[idx_w & this->mask_]);

        // mark the current push has done after initializing
        while (done_w_ != idx_w) {}
        done_w_.fetch_add(1, std::memory_order_acq_rel);
        return true;
    }

    /* directly construct an object at the end of the queue. */
    /* return false if the queue is full now, otherwise true. */
    template <typename... Args>
    bool emplace(Args&&... args) noexcept {
        // try to acquire a place for the current emplacement
        uint32_t idx_w = next_w_.load(std::memory_order_acquire);
        do {
            if (idx_w - done_r_ == this->size_) return false;
        } while (!next_w_.compare_exchange_weak(idx_w, idx_w + 1));

        new (&this->queue_[idx_w & this->mask_]) T(std::forward<Args>(args)...);

        // mark the current emplacement has done after writing
        while (done_w_ != idx_w) {}
        done_w_.fetch_add(1, std::memory_order_acq_rel);
        return true;
    }

    /* pop an object from the front of the queue, and handle it with the callback user provides. */
    /* make sure the handle is available for concurrently applied to a same element(e.g. copy it). */
    /* return false if the queue is empty now, otherwise true. */
    bool pop(PopHandle<T>&& handle) noexcept {
        // if another consumer has committed its manipulation on the element
        // retry to handle the next until the queue is empty
        uint32_t idx_r = done_r_.load(std::memory_order_acquire);
        do {
            if (idx_r == done_w_) return false;
            handle(this->queue_[idx_r & this->mask_]);
        } while (!done_r_.compare_exchange_weak(idx_r, idx_r + 1));

        return true;
    }

#ifndef NDEBUG
    /* enabled in debug mode to dump the content of the queue.*/
    void dump(std::string&& path) { BasicQueue<T>::dump(done_r_, done_w_, std::move(path)); }
#endif
};

}  // namespace lfcq
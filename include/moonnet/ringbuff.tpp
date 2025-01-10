/* BSD 3-Clause License

Copyright (c) 2024, MoonforDream

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Author: MoonforDream

*/

#ifndef _RINGBUFF_TPP_
#define _RINGBUFF_TPP_

namespace moon {

    /**
     * @brief Pushes an item to the head of the Ring Buffer.
     *
     * This function attempts to add a copy of the given `item` to the head of
     * the ring buffer. If the buffer is full, the push operation fails and
     * returns `false`. This operation is thread-safe and uses atomic operations
     * to manage concurrent access.
     *
     * @tparam T The type of elements stored in the ring buffer.
     * @param item The item to be pushed into the buffer. It is copied into the
     * buffer.
     *
     * @return `true` if the push operation was successful; `false` if the
     * buffer is full.
     */
    template <class T>
    bool ringbuff<T>::push(const T& item) {
        size_t head = head_.load(std::memory_order_relaxed);
        // capacity_ is a power of two, so &(capacity_-1) is equivalent to %
        // capacity_
        size_t nhead = (head + 1) & (capacity_ - 1);
        // Check if the buffer is full
        if (nhead == tail_.load(std::memory_order_acquire)) return false;
        buffer_[head] = item;
        head_.store(nhead, std::memory_order_release);
        return true;
    }

    /**
     * @brief Pushes an item to the head of the Ring Buffer using move
     * semantics.
     *
     * This function attempts to add the given `item` to the head of the ring
     * buffer by moving it. If the buffer is full, the push operation fails and
     * returns `false`. This operation is thread-safe and uses atomic operations
     * to manage concurrent access.
     *
     * @tparam T The type of elements stored in the ring buffer.
     * @param item The item to be moved into the buffer. It is moved into the
     * buffer to avoid copying.
     *
     * @return `true` if the push operation was successful; `false` if the
     * buffer is full.
     */
    template <class T>
    bool ringbuff<T>::push_move(T&& item) {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t nhead = (head + 1) & (capacity_ - 1);
        // Check if the buffer is full
        if (nhead == tail_.load(std::memory_order_acquire)) return false;
        buffer_[head] = std::move(item);
        head_.store(nhead, std::memory_order_release);
        return true;
    }

    /**
     * @brief Pops an item from the tail of the Ring Buffer.
     *
     * This function attempts to remove an item from the tail of the ring buffer
     * and store it in `item`. If the buffer is empty, the pop operation fails
     * and returns `false`. This operation is thread-safe and uses atomic
     * operations to manage concurrent access.
     *
     * @tparam T The type of elements stored in the ring buffer.
     * @param item A reference to the variable where the popped item will be
     * stored. The item is copied.
     *
     * @return `true` if the pop operation was successful; `false` if the buffer
     * is empty.
     */
    template <class T>
    bool ringbuff<T>::pop(T& item) {
        size_t tail = tail_.load(std::memory_order_relaxed);
        // Check if the buffer is empty
        if (tail == head_.load(std::memory_order_acquire)) return false;
        item = buffer_[tail];
        tail_.store((tail + 1) & (capacity_ - 1), std::memory_order_release);
        return true;
    }

    /**
     * @brief Pops an item from the tail of the Ring Buffer using move
     * semantics.
     *
     * This function attempts to remove an item from the tail of the ring buffer
     * and move it into `item`. If the buffer is empty, the pop operation fails
     * and returns `false`. This operation is thread-safe and uses atomic
     * operations to manage concurrent access.
     *
     * @tparam T The type of elements stored in the ring buffer.
     * @param item A reference to the variable where the popped item will be
     * moved. The item is moved.
     *
     * @return `true` if the pop operation was successful; `false` if the buffer
     * is empty.
     */
    template <class T>
    bool ringbuff<T>::pop_move(T& item) {
        size_t tail = tail_.load(std::memory_order_relaxed);
        // Check if the buffer is empty
        if (tail == head_.load(std::memory_order_acquire)) return false;
        item = std::move(buffer_[tail]);
        tail_.store((tail + 1) & (capacity_ - 1), std::memory_order_release);
        return true;
    }

    template <class T>
    bool ringbuff<T>::empty() {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

    template <class T>
    bool ringbuff<T>::full() {
        // return ((tail_.load(std::memory_order_acquire) + 1) % capacity_) ==
        // head_.load(std::memory_order_acquire);
        return ((head_.load(std::memory_order_acquire) + 1) &
                (capacity_ - 1)) == tail_.load(std::memory_order_acquire);
    }

    template <class T>
    void ringbuff<T>::swap(ringbuff& other) noexcept {
        head_.exchange(other.head_.load(std::memory_order_relaxed),
                       std::memory_order_relaxed);
        tail_.exchange(other.tail_.load(std::memory_order_relaxed),
                       std::memory_order_relaxed);
        std::swap(buffer_, other.buffer_);
    }

    /**
     * @brief Checks if a number is a power of two.
     *
     * This function determines whether the given number `n` is a power of two.
     *
     * @tparam T The type parameter for the ring buffer.
     * @param n The number to check.
     *
     * @return `true` if `n` is a non-zero power of two; otherwise, `false`.
     */
    template <class T>
    bool ringbuff<T>::is_powtwo(size_t n) const {
        return n && ((n & (n - 1)) == 0);
    }

    /**
     * @brief Calculates the next power of two greater than or equal to a given
     * number.
     *
     * This function finds the smallest power of two that is greater than or
     * equal to `n`.
     *
     * @tparam T The type parameter for the ring buffer.
     * @param n The input number.
     *
     * @return The next power of two greater than or equal to `n`. If `n` is 0,
     * returns 1.
     */
    template <class T>
    size_t ringbuff<T>::next_powtwo(size_t n) const {
        if (!n) return 1;
        --n;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        if (sizeof(size_t) > 4) n |= n >> 32;
        return n + 1;
    }

    /**
     * @brief Adjusts the size to the nearest power of two.
     *
     * This function adjusts the input `size` to the nearest power of two. If
     * `size` is already a power of two, it returns `size`; otherwise, it
     * returns the next higher power of two.
     *
     * @tparam T The type parameter for the ring buffer.
     * @param size The original size.
     *
     * @return The adjusted size, guaranteed to be a power of two.
     */
    template <class T>
    size_t ringbuff<T>::adj_size(size_t size) const {
        return is_powtwo(size) ? size : next_powtwo(size);
    }

    template <class T>
    size_t ringbuff<T>::capacity() const {
        return capacity_;
    }

    /**
     * @brief Check the number of existing members in the Ring-Buffer
     *
     * This function involves atomic operations, so it may cause performance
     * loss to some extent. It is recommended not to use it unless it is
     * absolutely necessary for the business
     *
     * @tparam T The type parameter for the ring buffer.
     *
     * @return the number of existing members in the Ring-Buffer
     */
    template <class T>
    size_t ringbuff<T>::size() const {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        if (head >= tail) {
            // 当 head 在 tail 前面或相等时，直接相减即可
            return head - tail;
        } else {
            // 当 head 绕过 buffer 结束循环回到前面时
            return capacity_ - tail + head;
        }
    }

    /**
     * @brief Moves all elements from the Ring Buffer to the provided std::list.
     *
     * This function transfers all elements currently stored in the Ring Buffer
     * to the specified `std::list<T>`. After the operation, the Ring Buffer is
     * marked as empty. It efficiently handles cases where the Ring Buffer wraps
     * around its internal buffer.
     *
     * @tparam T The type of elements stored in the Ring Buffer.
     * @param list_ The target std::list<T> where elements will be moved.
     */
    template <class T>
    void ringbuff<T>::swap_to_list(std::list<T>& list_) {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);

        if (tail == head) {
            return;  // 缓冲区为空
        }

        if (tail < head) {
            // 不需要绕回
            for (size_t i = tail; i < head; ++i) {
                list_.emplace_back(std::move(buffer_[i]));
            }
        } else {
            // 需要绕回
            for (size_t i = tail; i < capacity_; ++i) {
                list_.emplace_back(std::move(buffer_[i]));
            }
            for (size_t i = 0; i < head; ++i) {
                list_.emplace_back(std::move(buffer_[i]));
            }
        }

        // 更新 tail，标记缓冲区为空
        tail_.store(head, std::memory_order_release);
    }

    /**
     * @brief Moves all elements from the Ring Buffer to the provided
     * std::vector.
     *
     * This function transfers all elements currently stored in the Ring Buffer
     * to the specified `std::vector<T>`. After the operation, the Ring Buffer
     * is marked as empty. It efficiently handles cases where the Ring Buffer
     * wraps around its internal buffer.
     *
     * @tparam T The type of elements stored in the Ring Buffer.
     * @param vec_ The target std::vector<T> where elements will be moved.
     */
    template <class T>
    void ringbuff<T>::swap_to_vector(std::vector<T>& vec_) {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);

        if (tail == head) {
            return;  // 缓冲区为空
        }

        size_t size = (head + capacity_ - tail) % capacity_;
        vec_.reserve(vec_.size() + size);  // 预分配内存，减少重新分配

        if (tail < head) {
            // 不需要绕回
            std::move(buffer_ + tail, buffer_ + head, std::back_inserter(vec_));
        } else {
            // 需要绕回
            std::move(buffer_ + tail, buffer_ + capacity_,
                      std::back_inserter(vec_));
            std::move(buffer_, buffer_ + head, std::back_inserter(vec_));
        }

        // 更新 tail，标记缓冲区为空
        tail_.store(head, std::memory_order_release);
    }

    /**
     * @brief Moves all elements from the Ring Buffer to a new std::list.
     *
     * This function creates a new `std::list<T>`, moves all elements from the
     * Ring Buffer into this list, and returns it. After the operation, the Ring
     * Buffer is marked as empty. It efficiently handles cases where the Ring
     * Buffer wraps around its internal buffer.
     *
     * @tparam T The type of elements stored in the Ring Buffer.
     * @return std::list<T> A new list containing all elements from the Ring
     * Buffer.
     */
    template <class T>
    std::list<T> ringbuff<T>::swap_to_list() {
        std::list<T> list_;
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);

        if (tail == head) {
            return list_;  // 缓冲区为空
        }

        if (tail < head) {
            // 不需要绕回
            for (size_t i = tail; i < head; ++i) {
                list_.emplace_back(std::move(buffer_[i]));
            }
        } else {
            // 需要绕回
            for (size_t i = tail; i < capacity_; ++i) {
                list_.emplace_back(std::move(buffer_[i]));
            }
            for (size_t i = 0; i < head; ++i) {
                list_.emplace_back(std::move(buffer_[i]));
            }
        }

        // 更新 tail，标记缓冲区为空
        tail_.store(head, std::memory_order_release);
        return list_;
    }

    /**
     * @brief Moves all elements from the Ring Buffer to a new std::vector.
     *
     * This function creates a new `std::vector<T>`, moves all elements from the
     * Ring Buffer into this vector, and returns it. After the operation, the
     * Ring Buffer is marked as empty. It efficiently handles cases where the
     * Ring Buffer wraps around its internal buffer.
     *
     * @tparam T The type of elements stored in the Ring Buffer.
     * @return std::vector<T> A new vector containing all elements from the Ring
     * Buffer.
     */
    template <class T>
    std::vector<T> ringbuff<T>::swap_to_vector() {
        std::vector<T> vec_;
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);

        if (tail == head) {
            return vec_;  // 缓冲区为空
        }

        size_t size = (head + capacity_ - tail) % capacity_;
        vec_.reserve(size);  // 预分配内存，减少重新分配

        if (tail < head) {
            // 不需要绕回
            std::move(buffer_ + tail, buffer_ + head, std::back_inserter(vec_));
        } else {
            // 需要绕回
            std::move(buffer_ + tail, buffer_ + capacity_,
                      std::back_inserter(vec_));
            std::move(buffer_, buffer_ + head, std::back_inserter(vec_));
        }

        // 更新 tail，标记缓冲区为空
        tail_.store(head, std::memory_order_release);
        return vec_;
    }
}  // namespace moon

#endif

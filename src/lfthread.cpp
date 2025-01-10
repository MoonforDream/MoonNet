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

#include "lfthread.h"
#include "ringbuff.h"

using namespace moon;

/**
 * @brief Enqueues a task into the thread's buffer.
 *        Returns true if the task is successfully added, otherwise returns
 * false if the buffer is full.
 *
 * @param _task The task to be enqueued.
 * @return bool True if the task is enqueued successfully, false if the buffer
 * is full.
 */
bool lfthread::enqueue_task(task _task) {
    if (buffer_.push(_task)) {
        return true;
    } else {
        // buffer_ is full
        return false;
    }
}

/**
 * @brief Enqueues a task into the thread's buffer by moving it.
 *        Returns true if the task is successfully moved and added, otherwise
 * returns false if the buffer is full.
 *
 * @param _task The task to be enqueued, using move semantics to avoid copying.
 * @return bool True if the task is moved and enqueued successfully, false if
 * the buffer is full.
 */
bool lfthread::enqueue_task_move(task&& _task) {
    if (buffer_.push_move(std::move(_task))) {
        return true;
    } else {
        // buffer_ is full
        return false;
    }
}

/**
 * @brief Shuts down the thread by setting the shutdown flag and joining the
 * thread if it's joinable. Pushes an empty task to the buffer to wake up the
 * thread for shutdown.
 */
void lfthread::t_shutdown() {
    if (shutdown_) return;
    shutdown_ = true;
    // 推送一个空任务以唤醒线程退出
    buffer_.push([]() {});
    if (t_.joinable()) t_.join();
}

/**
 * @brief Main task function of the thread, executes tasks from the buffer.
 *        Implements a dynamic backoff strategy to manage polling for tasks.
 */
void lfthread::t_task() {
    // 动态退避策略
    auto min_sleep = std::chrono::milliseconds(1);
    auto max_sleep = std::chrono::milliseconds(100);
    auto cur_sleep = min_sleep;
    std::function<void()> _task;
    while (!shutdown_) {
        if (buffer_.pop_move(_task)) {
            _task();
            cur_sleep = min_sleep;
        } else {
            cur_sleep = cur_sleep * 2 >= max_sleep ? max_sleep : cur_sleep * 2;
            std::this_thread::sleep_for(cur_sleep);
        }
    }
    // execute task before shutdown
    while (buffer_.pop(_task)) {
        task();
    }
}

/**
 * @brief Returns the number of tasks currently in the buffer.
 *
 * @return int The number of tasks in the buffer.
 */
int lfthread::getload() const { return buffer_.size(); }

/**
 * @brief Swaps the internal buffer with another ring buffer.
 *
 * @param rb_ The ring buffer to swap with.
 */
void lfthread::swap_to_ringbuff(ringbuff<task>& rb_) { buffer_.swap(rb_); }

/**
 * @brief Swaps the internal buffer content to a std::list.
 *
 * @param list_ List to transfer tasks to.
 */
void lfthread::swap_to_list(std::list<lfthread::task>& list_) {
    buffer_.swap_to_list(list_);
}

/**
 * @brief Swaps the internal buffer content to a std::vector.
 *
 * @param vec_ Vector to transfer tasks to.
 */
void lfthread::swap_to_vector(std::vector<lfthread::task>& vec_) {
    buffer_.swap_to_vector(vec_);
}

/**
 * @brief Swaps the internal buffer content to a new std::list and returns it.
 *
 * @return std::list<lfthread::task> List containing all tasks swapped out from
 * the buffer.
 */
std::list<lfthread::task> lfthread::swap_to_list() {
    return buffer_.swap_to_list();
}

/**
 * @brief Swaps the internal buffer content to a new std::vector and returns it.
 *
 * @return std::vector<lfthread::task> Vector containing all tasks swapped out
 * from the buffer.
 */
std::vector<lfthread::task> lfthread::swap_to_vector() {
    return buffer_.swap_to_vector();
}

/**
 * @brief Swaps the internal buffer content to a new std::vector and returns it.
 *
 * @return std::vector<lfthread::task> Vector containing all tasks swapped out
 * from the buffer.
 */
lfthread::lfthread(lfthread&& other) noexcept
    : shutdown_(other.shutdown_), t_(std::move(other.t_)) {
    buffer_.swap(other.buffer_);
    // other.buffer_.swap(buffer_);
}

/**
 * @brief Move assignment operator for lfthread, transfers the state from
 * another thread object to this one. Properly shuts down the current thread
 * before moving the state.
 *
 * @param other The other lfthread object to move from.
 * @return lfthread& Reference to this updated lfthread object.
 */
lfthread& lfthread::operator=(lfthread&& other) noexcept {
    if (this != &other) {
        t_shutdown();
        buffer_.swap(other.buffer_);
        shutdown_ = other.shutdown_;
        t_ = std::move(other.t_);
    }
    return *this;
}

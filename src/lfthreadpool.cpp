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

#include "lfthreadpool.h"
#include <atomic>
#include <chrono>
#include "lfthread.h"

using namespace moon;

/**
 * @brief Constructor for lfthreadpool that initializes thread pool settings.
 *        Sets the number of threads(default is -1), buffer size per
 * thread(default is 1024), and the pooling mode(default is PoolMode::Static).
 *
 * @param tnum Number of threads in the pool. If less than or equal to 0, the
 * number is automatically determined.
 * @param buffsize Size of the task buffer for each thread.
 * @param mode Operation mode of the pool, either static or dynamic.
 */
lfthreadpool::lfthreadpool(int tnum, size_t buffsize, PoolMode mode)
    : shutdown_(false), mode_(mode), buffsize_(buffsize) {
    // 默认为-1,表示使用内置计算适合线程数
    if (tnum <= 0)
        setnum();
    else
        setnum(tnum);
    init();
}

/**
 * @brief Destructor for lfthreadpool, ensures all threads are properly shut
 * down and cleaned up.
 */
lfthreadpool::~lfthreadpool() {
    t_shutdown();
    for (auto &t : workers_) {
        delete t;
    }
    workers_.clear();
    tnum_ = 0;
}

/**
 * @brief Initializes the thread pool by creating threads and setting up any
 * necessary infrastructure based on the mode.
 */
void lfthreadpool::init() {
    for (int i = 0; i < tnum_; ++i) {
        workers_.emplace_back(new lfthread(buffsize_));
    }
    if (mode_ == PoolMode::Dynamic) {
        mentor_ = std::thread([this] { this->adjust_task(); });
    }
}

/**
 * @brief Determines the next thread index for assigning a task based on the
 * pool mode.
 *
 * @return const size_t Index of the next thread to which a task will be
 * assigned.
 */
const size_t lfthreadpool::getnext() {
    int idx = 0;
    if (mode_ == PoolMode::Dynamic) {
        idx = getminidx();
    } else if (mode_ == PoolMode::Static) {
        next_ = (next_ + 1) % tnum_;
        idx = next_;
    }
    return idx;
}

/**
 * @brief Shuts down the thread pool and joins all threads to ensure clean exit.
 */
void lfthreadpool::t_shutdown() {
    if (shutdown_.exchange(true, std::memory_order_acquire)) return;
    if (mode_ == PoolMode::Dynamic && mentor_.joinable()) mentor_.join();
    for (int i = 0; i < tnum_; ++i) {
        workers_[i]->t_shutdown();
    }
}

/**
 * @brief Removes the most heavily loaded thread, redistributes its tasks, and
 * shuts it down.
 */
void lfthreadpool::del_thread_dispath() {
    int idx = getmaxidx();

    ringbuff<lfthread::task> rb(buffsize_);
    workers_[idx]->swap_to_ringbuff(rb);
    workers_[idx]->t_shutdown();

    workers_[idx] = std::move(workers_.back());
    workers_.pop_back();
    tnum_.fetch_sub(1, std::memory_order_relaxed);

    lfthread::task _t;
    while (rb.pop_move(_t)) {
        add_task(std::move(_t));
    }
}

/**
 * @brief Adds a new thread to the pool to handle increased load.
 */
void lfthreadpool::add_thread() {
    workers_.emplace_back(new lfthread(buffsize_));
    tnum_.fetch_add(1, std::memory_order_relaxed);
}

/**
 * @brief Periodically adjusts the number of threads in the pool based on the
 * load.
 */
void lfthreadpool::adjust_task() {
    while (shutdown_) {
        std::this_thread::sleep_for(std::chrono::seconds(timesec_));
        if (shutdown_) break;
        int num = tnum_.load(std::memory_order_acquire);
        if (num > min_tnum && num < max_tnum) {
            int load = getload();
            if (load < load_min) {
                del_thread_dispath();
                timesec_ += coolsec_;
            } else if (load > load_max) {
                add_thread();
                timesec_ -= coolsec_;
            }
        }
        if (timesec_ < ADJUST_TIMEOUT_SEC) timesec_ = ADJUST_TIMEOUT_SEC;
    }
}

/**
 * @brief Gets the index of the thread with the minimum load.
 *
 * @return const size_t Index of the least loaded thread.
 */
const size_t lfthreadpool::getminidx() {
    int min_load = workers_[0]->getload();
    int idx = 0;
    int size = tnum_.load(std::memory_order_acquire);
    for (int i = 1; i < size; ++i) {
        int cur_load = workers_[i]->getload();
        if (cur_load < min_load) {
            min_load = cur_load;
            idx = i;
        }
    }
    return idx;
}

/**
 * @brief Gets the index of the thread with the maximum load.
 *
 * @return const size_t Index of the most loaded thread.
 */
const size_t lfthreadpool::getmaxidx() {
    int max_load = workers_[0]->getload();
    int idx = 0;
    int size = tnum_.load(std::memory_order_acquire);
    for (int i = 1; i < size; ++i) {
        int cur_load = workers_[i]->getload();
        if (cur_load > max_load) {
            max_load = cur_load;
            idx = i;
        }
    }
    return idx;
}

/**
 * @brief Computes the average load across all threads in the pool.
 *
 * @return const int Average load expressed as a percentage.
 */
const int lfthreadpool::getload() {
    int sum = 0, avg_load = 0;
    int size = tnum_.load(std::memory_order_acquire);
    for (int i = 0; i < size; ++i) {
        sum += workers_[i]->getload();
    }
    if (sum == 0) return 0;
    avg_load = sum / size;
    return (avg_load / sum) * 100;
}

/**
 * @brief Sets the number of threads in the pool based on system resources.
 */
void lfthreadpool::setnum() {
    tnum_ = getcores();
    max_tnum = tnum_ * 2 - 1;
    min_tnum = tnum_;
}

/**
 * @brief Sets the number of threads in the pool based on a specified value.
 *
 * @param n Number of threads to be set in the pool.
 */
void lfthreadpool::setnum(size_t n) {
    tnum_ = n;
    max_tnum = n * 2 - 1;
    min_tnum = n;
}

/**
 * @brief Retrieves the number of processor cores available and adjusts for
 * threading purposes.
 *
 * @return int Number of logical cores available for the thread pool.
 */
int lfthreadpool::getcores() const {
    unsigned int cores = std::thread::hardware_concurrency() / 2;
    if (cores == 0) cores = 4;
    return cores + 1;
}

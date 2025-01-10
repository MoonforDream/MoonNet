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

#include "looptpool.h"
#include "eventloop.h"
#include "loopthread.h"

using namespace moon;

looptpool::looptpool(eventloop* base, bool dispath)
    : baseloop_(base), dispath_(dispath) {
    if (dispath_) {
        init_adjust();
    }
};

looptpool::~looptpool() {
    stop();
    for (auto& t : loadvec_) {
        delete t->getbaseloop();
    }
    loadvec_.clear();
    t_num = 0;
}

/**
 * @brief Initializes the loop pool with a specified timeout.
 *
 * Creates loop threads based on the current loop count (`t_num`) and
 * initializes their event loops.
 *
 * @param timeout The timeout value for each loop thread in milliseconds.
 */
void looptpool::init_pool(int timeout) {
    timeout_ = timeout;
    for (int i = 0; i < t_num; ++i) {
        loopthread* lt = new loopthread(timeout);
        loadvec_.emplace_back(lt->getloop());
    }
}

/**
 * @brief Creates a loop pool with automatic adjustment enabled.
 *
 * Sets the loop count based on internal logic, initializes the pool with the
 * given timeout, and starts the adjustment mechanism.
 *
 * @param timeout The timeout value for each loop thread in milliseconds.
 */
void looptpool::create_pool(int timeout) {
    dispath_ = true;
    settnum();
    init_pool(timeout);
    init_adjust();
}

/**
 * @brief Creates a loop pool with a specified number of loops and automatic
 * adjustment enabled.
 *
 * Sets the loop count to `n`, initializes the pool with the given timeout, and
 * starts the adjustment mechanism.
 *
 * @param n The number of event loops to create.
 * @param timeout The timeout value for each loop thread in milliseconds.
 */
void looptpool::create_pool(int n, int timeout) {
    dispath_ = true;
    settnum(n);
    init_pool(timeout);
    init_adjust();
}

/**
 * @brief Creates a loop pool without enabling automatic adjustment.
 *
 * Sets the loop count to `n`, initializes the pool with the given timeout, and
 * disables the adjustment mechanism.
 *
 * @param n The number of event loops to create.
 * @param timeout The timeout value for each loop thread in milliseconds.
 */
void looptpool::create_pool_noadjust(int n, int timeout) {
    settnum_noadjust(n);
    init_pool(timeout);
    stop_adjust();
}

/**
 * @brief Dispatches an event to an appropriate event loop.
 *
 * Selects an event loop based on the current dispatching strategy. If
 * dispatching is enabled, it selects the loop with the minimum load. Otherwise,
 * it uses a round-robin approach.
 *
 * @return Pointer to the selected `eventloop` instance.
 */
eventloop* looptpool::ev_dispatch() {
    if (t_num == 0) return baseloop_;
    if (dispath_) {
        return getminload();
    } else {
        next_ = (next_ + 1) % t_num;
        return loadvec_[next_];
    }
}

/**
 * @brief Deletes the event loop with the maximum load and redistributes its
 * events.
 *
 * Identifies the loop with the highest load, retrieves all its events, and
 * reassigns them to other loops using the dispatch mechanism. The overloaded
 * loop is then removed from the pool.
 */
void looptpool::delloop_dispatch() {
    int idx = getmaxidx();
    eventloop* ep = loadvec_[idx];
    std::list<event*> list;
    ep->getallev(list);
    for (auto& ev : list) {
        ev_dispatch()->add_event(ev);
    }
    loadvec_[idx] = std::move(loadvec_.back());
    loadvec_.pop_back();
    delete ep->getbaseloop();
    --t_num;
}

/**
 * @brief Adds a new event loop to the pool.
 *
 * Creates a new loop thread with the current timeout value, adds its event loop
 * to the pool, and increments the loop count.
 */
void looptpool::addloop() {
    loopthread* lt = new loopthread(timeout_);
    loadvec_.emplace_back(lt->getloop());
    ++t_num;
}

/**
 * @brief Calculates the average load scale of all event loops.
 *
 * Sums up the load of each event loop and computes the average load as a
 * percentage.
 *
 * @return The average load scale as an integer percentage.
 */
int looptpool::getscale() {
    int sum = 0, avg_scale = 0;
    for (auto& ep : loadvec_) {
        sum += ep->getload();
    }
    if (sum == 0) return 0;
    avg_scale = sum / t_num;
    return (avg_scale / sum) * 100;
}

/**
 * @brief Periodically adjusts the number of event loops based on the current
 * load.
 *
 * Continuously monitors the load and adjusts the pool size by adding or
 * removing loops if the average load falls below or exceeds predefined
 * thresholds. This function runs in a separate thread when dispatching is
 * enabled.
 */
void looptpool::adjust_task() {
    while (dispath_) {
        std::this_thread::sleep_for(std::chrono::seconds(timesec_));
        if (!dispath_) break;
        if (t_num > min_tnum && t_num < max_tnum) {
            int scale = getscale();
            if (scale < scale_min) {
                delloop_dispatch();
                timesec_ += coolsec_;
            } else if (scale > scale_max) {
                addloop();
                timesec_ -= coolsec_;
            }
        }
        if (timesec_ < ADJUST_TIMEOUT_SEC) timesec_ = ADJUST_TIMEOUT_SEC;
    }
}

/**
 * @brief Enables the automatic adjustment of the event loop pool.
 *
 * If the adjustment manager thread is not already running, it sets the dispatch
 * flag to `true` and initializes the adjustment mechanism.
 */
void looptpool::enable_adjust() {
    if (!manager_.joinable()) {
        dispath_ = true;
        init_adjust();
    }
}

/**
 * @brief Retrieves the event loop with the minimum current load.
 *
 * Iterates through all event loops in the pool and identifies the one with the
 * least load.
 *
 * @return Pointer to the `eventloop` instance with the minimum load.
 */
eventloop* looptpool::getminload() {
    int min_load = loadvec_[0]->getload();
    int idx = 0;
    unsigned int size = t_num;
    for (int i = 1; i < size; ++i) {
        int cur_load = loadvec_[i]->getload();
        if (cur_load < min_load) {
            min_load = cur_load;
            idx = i;
        }
    }
    return loadvec_[idx];
}

/**
 * @brief Identifies the index of the event loop with the maximum load.
 *
 * Iterates through all event loops in the pool and finds the index of the one
 * with the highest load.
 *
 * @return The index of the `eventloop` instance with the maximum load.
 */
int looptpool::getmaxidx() {
    int max_load = loadvec_[0]->getload();
    int idx = 0;
    unsigned int size = t_num;
    for (int i = 1; i < size; ++i) {
        int cur_load = loadvec_[i]->getload();
        if (cur_load > max_load) {
            max_load = cur_load;
            idx = i;
        }
    }
    return idx;
}

/**
 * @brief Stops all event loops and cleans up resources.
 *
 * Iterates through all event loops in the pool, signals them to stop, and joins
 * their threads. It also stops the adjustment manager thread if it is running.
 */
void looptpool::stop() {
    for (auto& ep : loadvec_) {
        ep->loopbreak();
        ep->getbaseloop()->join();
    }
    if (manager_.joinable()) {
        stop_adjust();
        manager_.join();
    }
}

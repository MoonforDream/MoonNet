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

#include <sys/timerfd.h>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include "eventloop.h"
#include "event.h"
#include "timerevent.h"

using namespace moon;

/**
 * @brief Constructs a new `timerevent` instance.
 *
 * Initializes the `timerevent` with the provided event loop, timeout in
 * milliseconds, and periodic flag. It calls the private `_init_()` function to
 * set up the timer file descriptor and register it with the event loop.
 *
 * @param loop Pointer to the associated `eventloop`.
 * @param timeout_ms The timeout duration in milliseconds after which the timer
 * event is triggered.
 * @param periodic A boolean flag indicating whether the timer should trigger
 * periodically.
 */
timerevent::timerevent(eventloop *loop, int timeout_ms, bool periodic)
    : loop_(loop), timeout_ms_(timeout_ms), periodic_(periodic) {
    _init_();
}

/**
 * @brief Destructs the `timerevent` instance.
 *
 * Cleans up by deleting the associated `event`, closing the timer file
 * descriptor, and resetting internal pointers to prevent dangling references.
 */
timerevent::~timerevent() {
    if (ev_) {
        delete ev_;
        ev_ = nullptr;
    }
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

/**
 * @brief Initializes the timer file descriptor and registers it with the event
 * loop.
 *
 * Creates a non-blocking timer file descriptor using `timerfd_create` and
 * configures it with the specified timeout and periodicity using
 * `timerfd_settime`. It then creates an associated `event` to listen for read
 * events on the timer file descriptor and sets the callback for handling
 * timeout events.
 *
 * @details
 * The `itimerspec` structure is used to specify the initial expiration
 * (`it_value`) and the interval between expirations (`it_interval`). If the
 * timer is periodic, `it_interval` is set to the same value as `it_value`;
 * otherwise, it is set to zero, making the timer a one-shot timer.
 */
void timerevent::_init_() {
    fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd_ == -1) {
        perror("timerfd create");
        exit(EXIT_FAILURE);
    }

    itimerspec tvalue;
    // it_value设置定时器第一次到期时间
    tvalue.it_value.tv_sec = timeout_ms_ / 1000;  // 初始化到期秒数
    tvalue.it_value.tv_nsec =
        (timeout_ms_ % 1000) * 1000000;  // 初始化到期纳秒数
    tvalue.it_interval = {0, 0};
    // it_interval设置定时器第一次之后每次到期的间隔时间，设置为0,定时器只会触发一次,非0为周期性触发
    if (periodic_) {
        tvalue.it_interval.tv_sec = timeout_ms_ / 1000;  // 间隔时间秒数
        tvalue.it_interval.tv_nsec =
            (timeout_ms_ % 1000) * 1000000;  // 间隔时间的纳秒数
    }

    if (timerfd_settime(fd_, 0, &tvalue, NULL) == -1) {
        perror("timerfd_settime error");
        ::close(fd_);
        exit(EXIT_FAILURE);
    }
    ev_ = new event(loop_, fd_, EPOLLIN);
    ev_->setcb(std::bind(&timerevent::handle_timeout, this), nullptr, nullptr);
}

int timerevent::getfd() const { return fd_; }

eventloop *timerevent::getloop() const { return loop_; }

void timerevent::setcb(const moon::timerevent::Callback &cb) { cb_ = cb; }

void timerevent::enable_listen() {
    if (ev_) {
        ev_->enable_listen();
    }
}

void timerevent::del_listen() {
    if (ev_) ev_->del_listen();
}

/**
 * @brief Handles the timer timeout event.
 *
 * Reads the number of expirations from the timer file descriptor to acknowledge
 * the event. If a callback is set, it invokes the callback function.
 */
void timerevent::handle_timeout() {
    uint64_t exp;
    ssize_t s = read(fd_, &exp, sizeof(exp));
    if (s != sizeof(exp)) {
        perror("read timerfd");
        return;
    }
    if (cb_) cb_();
}

void timerevent::disable_cb() { cb_ = nullptr; }

timerevent::Callback timerevent::getcb() { return cb_; }

void timerevent::update_ep() {
    if (ev_) {
        ev_->update_ep();
    }
}

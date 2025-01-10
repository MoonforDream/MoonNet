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

// signalevent.cpp

#include "signalevent.h"
#include "event.h"
#include "eventloop.h"
#include <cstring>

using namespace moon;

signalevent* signalevent::sigev_ = nullptr;

/**
 * @brief Constructs a new `signalevent` instance.
 *
 * Initializes the `signalevent` with the provided event loop. It creates a pipe
 * for inter-thread communication, sets both ends of the pipe to non-blocking
 * mode, creates an associated `event` to listen for read events on the pipe's
 * read end, and sets up the callback for handling read events.
 *
 * @param base Pointer to the associated `eventloop`.
 */
signalevent::signalevent(eventloop* base) : loop_(base) {
    // 创建管道
    if (pipe(pipe_fd_) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // 设置管道为非阻塞模式
    setnonblock(pipe_fd_[0]);
    setnonblock(pipe_fd_[1]);

    ev_ = new event(loop_, pipe_fd_[0], EPOLLIN);
    ev_->setcb(std::bind(&signalevent::handle_read, this), nullptr, nullptr);
    //    enable_listen();

    // 保存实例指针，供信号处理函数使用
    sigev_ = this;
}

/**
 * @brief Destructs the `signalevent` instance.
 *
 * Cleans up by deleting the associated `event`, closing both ends of the pipe,
 * and resetting the static instance pointer.
 */
signalevent::~signalevent() {
    if (ev_) {
        delete ev_;
        ev_ = nullptr;
    }
    ::close(pipe_fd_[0]);
    ::close(pipe_fd_[1]);
    sigev_ = nullptr;
}

eventloop* signalevent::getloop() const { return loop_; }

/**
 * @brief Adds a signal to be monitored by the `signalevent`.
 *
 * Sets up the signal handler for the specified signal number (`signo`) using
 * `sigaction`. The handler is set to the static `handle_signal` function.
 *
 * @param signo The signal number to monitor (e.g., `SIGINT`, `SIGTERM`).
 */
void signalevent::add_signal(int signo) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signalevent::handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(signo, &sa, nullptr) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Adds multiple signals to be monitored by the `signalevent`.
 *
 * Iterates over the provided vector of signal numbers and sets up handlers for
 * each using `add_signal(int)`.
 *
 * @param signals A vector of signal numbers to monitor (e.g., `{SIGINT,
 * SIGTERM}`).
 */
void signalevent::add_signal(const std::vector<int>& signals) {
    for (auto sig : signals) {
        add_signal(sig);
    }
}

void signalevent::setcb(const Callback& cb) { cb_ = cb; }

void signalevent::enable_listen() {
    if (ev_) ev_->enable_listen();
}

void signalevent::del_listen() {
    if (ev_) ev_->del_listen();
}

void signalevent::handle_signal(int signo) {
    // 在信号处理函数中，向管道写入信号编号
    uint8_t sig = static_cast<uint8_t>(signo);
    if (sigev_) {
        ssize_t n = write(sigev_->pipe_fd_[1], &sig, sizeof(sig));
        (void)n;  // 忽略写入的返回值
    }
}

void signalevent::handle_read() {
    uint8_t signals[1024];
    ssize_t n = read(pipe_fd_[0], signals, sizeof(signals));
    if (n > 0) {
        for (ssize_t i = 0; i < n; ++i) {
            int sig = signals[i];
            if (cb_) cb_(sig);
        }
    }
}

void signalevent::disable_cb() { cb_ = nullptr; }

signalevent::Callback signalevent::getcb() { return cb_; }

void signalevent::update_ep() {
    if (ev_) {
        ev_->update_ep();
    }
}

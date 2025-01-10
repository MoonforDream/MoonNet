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

#include "base_event.h"
#include "eventloop.h"
#include "event.h"

using namespace moon;

/**
 * @brief Constructs a new `eventloop` instance.
 *
 * Initializes the event loop with the provided `loopthread`, timeout value, and
 * sets up the epoll instance. It also creates an event file descriptor for
 * inter-thread communication.
 *
 * @param base Pointer to the associated `loopthread`.
 * @param timeout The timeout value for epoll_wait in milliseconds.
 */
eventloop::eventloop(loopthread* base, int timeout)
    : baseloop_(base),
      timeout_(timeout),
      shutdown_(false),
      load_(0),
      events_(MAX_EVENTS) {
    epfd_ = epoll_create1(0);
    if (-1 == epfd_) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    create_eventfd();
}

eventloop::~eventloop() {
    loopbreak();
    for (auto& ev : evlist_) {
        delete ev;
    }
    for (auto& ev : delque_) {
        delete ev;
    }
    evlist_.clear();
    delque_.clear();
    close(epfd_);
    close(eventfd_);
}

loopthread* eventloop::getbaseloop() {
    if (!baseloop_) return nullptr;
    return baseloop_;
}

int eventloop::getefd() const { return epfd_; }

int eventloop::getevfd() const { return eventfd_; }

int eventloop::getload() const { return load_; }

void eventloop::add_event(event* event) {
    int fd = event->getfd();
    struct epoll_event ev;
    ev.data.ptr = event;
    // ev.data.fd=fd;
    ev.events = event->getevents();

    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl add error");
    }
    updateload(1);
    evlist_.emplace_back(event);
}

void eventloop::del_event(event* event) {
    int fd = event->getfd();

    if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
        perror("epoll_ctl del error");
    }
    updateload(-1);
    evlist_.remove(event);
}

void eventloop::mod_event(event* event) {
    int fd = event->getfd();
    struct epoll_event ev;
    ev.data.ptr = event;
    ev.events = event->getevents();

    if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
        perror("epoll_ctl modify error");
    }
}

/**
 * @brief Starts the event loop.
 *
 * Continuously waits for events using `epoll_wait`, handles triggered events,
 * resizes the events vector if necessary, and processes pending deletions.
 */
void eventloop::loop() {
    while (!shutdown_) {
        int n = epoll_wait(epfd_, events_.data(), MAX_EVENTS, timeout_);
        if (-1 == n) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < n; ++i) {
            auto ev = static_cast<event*>(events_[i].data.ptr);
            ev->setrevents(events_[i].events);
            ev->handle_cb();
        }
        if (n == events_.size()) {
            events_.resize(events_.size() * 2);
        }
        if (!delque_.empty()) {
            for (auto ev : delque_) {
                delete ev;
            }
            delque_.clear();
        }
    }
}

void eventloop::loopbreak() {
    if (shutdown_) return;
    write_eventfd();
}

/**
 * @brief Retrieves all currently active events.
 *
 * Swaps the provided `list` with the internal event list, effectively
 * transferring ownership.
 *
 * @param list Reference to a `std::list` that will receive the active events.
 */
void eventloop::getallev(std::list<event*>& list) { list.swap(evlist_); }

void eventloop::create_eventfd() {
    eventfd_ = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (eventfd_ < 0) {
        perror("eventfd create error");
        exit(EXIT_FAILURE);
    }
    event* ev = new event(this, eventfd_, EPOLLIN);
    ev->setcb(std::bind(&eventloop::read_eventfd, this), NULL, NULL);
    add_event(ev);
}

void eventloop::read_eventfd() {
    uint64_t opt = 1;
    ssize_t n = read(eventfd_, &opt, sizeof(opt));
    shutdown_ = true;
    if (n < 0) {
        if (errno == EINTR || errno == EAGAIN) {
            return;
        }
        perror("eventfd read error");
        exit(EXIT_FAILURE);
    }
}

void eventloop::write_eventfd() {
    uint64_t opt = 1;
    ssize_t n = write(eventfd_, &opt, sizeof(opt));
    if (n < 0) {
        if (errno == EINTR) {
            return;
        }
        perror("eventfd write error");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Adds an event to the pending deletion queue.
 *
 * Queues the specified `base_event` for deletion after the current event loop
 * iteration.
 *
 * @param ev Pointer to the `base_event` to be deleted.
 */
void eventloop::add_pending_del(base_event* ev) { delque_.emplace_back(ev); }

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

#include "event.h"
#include "eventloop.h"

using namespace moon;

event::event(eventloop *base, int fd, uint32_t events)
    : loop_(base), fd_(fd), events_(events) {}

event::~event() {}

int event::getfd() const { return fd_; }

uint32_t event::getevents() const { return events_; }

eventloop *event::getloop() const { return loop_; }

void event::setcb(const Callback &rcb, const Callback &wcb,
                  const Callback &ecb) {
    readcb_ = rcb;
    writecb_ = wcb;
    eventcb_ = ecb;
}

void event::setrcb(const Callback &rcb) { readcb_ = rcb; }

void event::setwcb(const Callback &wcb) { writecb_ = wcb; }

void event::setecb(const Callback &ecb) { eventcb_ = ecb; }

void event::setrevents(const uint32_t revents) { revents_ = revents; }

void event::update_ep() { loop_->mod_event(this); }

void event::enable_events(uint32_t op) {
    events_ |= op;
    update_ep();
}

void event::disable_events(uint32_t op) {
    events_ &= ~op;
    update_ep();
}

bool event::readable() { return (events_ & EPOLLIN); }

bool event::writeable() { return (events_ & EPOLLOUT); }

void event::enable_read() { enable_events(EPOLLIN); }

void event::disable_read() { disable_events(EPOLLIN); }

void event::enable_write() { enable_events(EPOLLOUT); }

void event::disable_write() { disable_events(EPOLLOUT); }

void event::enable_ET() { enable_events(EPOLLET); }

void event::disable_ET() { disable_events(EPOLLET); }

void event::reset_events() {
    events_ = 0;
    update_ep();
}

void event::del_listen() { loop_->del_event(this); }

void event::enable_listen() { loop_->add_event(this); }

void event::disable_cb() {
    readcb_ = nullptr;
    writecb_ = nullptr;
    eventcb_ = nullptr;
}

void event::handle_cb() {
    if ((revents_ & EPOLLIN) || (revents_ & EPOLLRDHUP) ||
        (revents_ & EPOLLPRI)) {
        if (readcb_) readcb_();
    }
    if (revents_ & EPOLLOUT) {
        if (writecb_) writecb_();
    }
    if (revents_ & (EPOLLERR | EPOLLHUP)) {
        if (eventcb_) eventcb_();
    }
}

event::Callback event::getrcb() { return readcb_; }

event::Callback event::getwcb() { return writecb_; }

event::Callback event::getecb() { return eventcb_; }

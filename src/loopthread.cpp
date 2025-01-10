// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "loopthread.h"
#include "eventloop.h"

using namespace moon;

loopthread::~loopthread() {
    if (loop_) {
        loop_->loopbreak();
        if (t_.joinable()) t_.join();
        delete loop_;
    } else {
        if (t_.joinable()) t_.join();
    }
}

void loopthread::_init_() {
    eventloop *loop = new eventloop(this, timeout_);
    {
        std::unique_lock<std::mutex> lock(mx_);
        loop_ = loop;
        cv_.notify_all();
    }
    loop_->loop();
}

eventloop *loopthread::getloop() {
    eventloop *ep = nullptr;
    {
        std::unique_lock<std::mutex> lock(mx_);
        cv_.wait(lock, [&]() { return loop_ != nullptr; });
        ep = loop_;
    }
    return ep;
}

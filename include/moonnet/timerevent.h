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

#ifndef _TIMEREVENT_H_
#define _TIMEREVENT_H_

#include "base_event.h"
#include <functional>

/** 弃用api
 * start->enable_listen
 * stop->del_listen
 **/

namespace moon {

    class event;
    class eventloop;

    class timerevent : public base_event {
    public:
        using Callback = std::function<void()>;
        /**
         * @brief 构造函数
         * @param loop 关联的事件循环
         * @param timeout_ms 定时器超时时间（毫秒）
         * @param periodic 是否为周期性定时器
         * @param cb 定时器回调函数
         */
        timerevent(eventloop* loop, int timeout_ms, bool periodic);
        ~timerevent();
        int getfd() const;
        eventloop* getloop() const override;
        void setcb(const Callback& cb);
        /* void start();
        void stop(); */
        void close() override { del_listen(); }
        void disable_cb() override;
        Callback getcb();
        /** v1.0.1 **/
        void update_ep() override;
        void enable_listen() override;
        void del_listen() override;

    private:
        void _init_();
        void handle_timeout();

    private:
        eventloop* loop_;
        event* ev_;
        int fd_;
        int timeout_ms_;
        bool periodic_;
        Callback cb_;
    };
}  // namespace moon

#endif

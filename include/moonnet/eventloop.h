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

#ifndef _EVENTLOOP_H_
#define _EVENTLOOP_H_

#include "wrap.h"
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <functional>
#include <vector>
#include <list>
#include <unistd.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>

#define MAX_EVENTS 65536

namespace moon {

    class base_event;
    class event;
    class loopthread;

    // reactor类-->事件循环类
    class eventloop {
    public:
        using Callback = std::function<void()>;
        eventloop(loopthread* base = nullptr, int timeout = -1);
        ~eventloop();
        loopthread* getbaseloop();
        int getefd() const;
        int getevfd() const;
        int getload() const;
        // 事件控制函数
        void add_event(event* event);
        void del_event(event* event);
        void mod_event(event* event);

        // void loop(struct timeval *tv);
        void loop();
        void loopbreak();
        void getallev(std::list<event*>& list);

        void create_eventfd();  // 创建通知文件描述符
        void read_eventfd();
        void write_eventfd();

        void add_pending_del(base_event* ev);

    private:
        // 更新负载
        void updateload(int n) { load_ += n; }

    private:
        int epfd_;
        int eventfd_;
        int timeout_ = -1;
        std::atomic<int> load_;
        std::atomic<bool> shutdown_;
        std::list<event*> evlist_;
        std::vector<epoll_event> events_;
        std::vector<base_event*> delque_;
        loopthread* baseloop_;
    };
}  // namespace moon

#endif

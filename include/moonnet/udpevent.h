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

// udpevent.h
#ifndef _UDPEVENT_H_
#define _UDPEVENT_H_

#include "base_event.h"
#include "buffer.h"
#include <functional>
#include <unistd.h>
#include <arpa/inet.h>

namespace moon {

    class eventloop;
    class event;

    // UDP 处理类
    class udpevent : public base_event {
    public:
        using Callback = std::function<void()>;
        using RCallback = std::function<void(const sockaddr_in&, udpevent*)>;
        udpevent(eventloop* base, int port);
        ~udpevent();

        buffer* getinbuff();
        eventloop* getloop() const override;
        void setcb(const RCallback& rcb, const Callback& ecb);
        void setrcb(const RCallback& rcb);
        void setecb(const Callback& ecb);
        void init_sock(int port);
        void enable_listen() override;  // 开始监听
        void del_listen() override;     // 停止监听
        void update_ep() override;      // 更新监听事件
        size_t receive(char* data, size_t len);
        std::string receive(size_t len);
        std::string receive();
        void send_to(const std::string& data,
                     const sockaddr_in& addr);  // 发送数据到指定地址

        void enable_read();
        void disable_read();
        void enable_ET();
        void disable_ET();
        void disable_cb() override;
        RCallback getrcb();
        Callback getecb();
        void close() override { del_listen(); }

    private:
        void handle_receive();  // 处理接收事件
    private:
        eventloop* loop_;
        int fd_;
        event* ev_;
        buffer inbuff_;
        RCallback receive_cb_;
        Callback event_cb_;
        bool started_ = false;
    };

}  // namespace moon

#endif

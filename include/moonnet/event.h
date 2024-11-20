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


#ifndef _EVENT_H_
#define _EVENT_H_

#include "base_event.h"
#include "wrap.h"
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <functional>
#include <list>
#include <unistd.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>

namespace moon{

class eventloop;


//事件类
class event:public base_event{
public:
    using Callback=std::function<void()>;
    event(eventloop *base,int fd,uint32_t events);
    ~event();
    int getfd() const;  //获取事件文件描述符
    uint32_t getevents() const;     //获取获取监听事件类型
    eventloop* getloop() const override;
    //设置回调函数，不需要传null即可
    void setcb(const Callback &rcb,const Callback &wcb,const Callback &ecb);
    void setrcb(const Callback &rcb);
    void setwcb(const Callback &wcb);
    void setecb(const Callback &ecb);
    Callback getrcb();
    Callback getwcb();
    Callback getecb();
    void setrevents(const uint32_t revents);    //设置触发事件类型
    void enable_events(uint32_t op);  //添加监听事件类型
    void disable_events(uint32_t op); //取消监听事件类型
    void update_ep() override;   //更新监听事件
    void handle_cb();   //处理事件回调函数
    
    bool readable();
    bool writeable();
    void enable_read();
    void disable_read();
    void enable_write();
    void disable_write();
    void enable_ET();
    void disable_ET();
    void reset_events();
    void del_listen() override;  //取消监听
    void enable_listen() override;   //开启监听
    void disable_cb() override;
    void close() override{
        del_listen();
    }
private:
    eventloop *loop_;
    int fd_;
    uint32_t events_;   //监听事件
    uint32_t revents_;  //触发事件
    Callback readcb_;   //读事件回调函数
    Callback writecb_;  //写事件回调函数
    Callback eventcb_;  //出了读写事件的其他事件触发回调函数,用作错误事件回调
};

}


#endif

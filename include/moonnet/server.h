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


#ifndef _SERVER_H_
#define _SERVER_H_

#include "base_event.h"
#include "acceptor.h"
#include "looptpool.h"
#include "eventloop.h"
#include "bfevent.h"
#include <functional>
#include <mutex>


namespace moon {

class udpevent;
class signalevent;
class timerevent;

class server{

public:
    using SCallback=std::function<void(int)>;
    using UCallback=std::function<void(const sockaddr_in&,udpevent*)>;
    using RCallback=std::function<void(bfevent*)>;
    using Callback=std::function<void()>;
    server(int port=-1);
    ~server();
    void start();       //启动
    void stop();    //停止
    void init_pool(int timeout=-1);   //初始化线程池
    void init_pool(int tnum,int timeout);   //以指定线程初始化线程池
    void init_pool_noadjust(int tnum,int timeout);  //不进行调度管理的指定线程初始化线程池
    void enable_tcp(int port);      //启用tcp服务
    void enable_tcp_accept();   //开启tcp连接监听器
    void disable_tcp_accept();  //取消tcp连接监听
    eventloop* getloop();
    //分发事件,建议先初始化线程池
    eventloop* dispatch();
    
    //设置tcp连接的回调函数
    void set_tcpcb(const RCallback &rcb,const Callback &wcb,const Callback &ecb);
    //对事件操作
    void addev(base_event *ev);
    void delev(base_event *ev);
    void modev(base_event *ev);
    //udpevent推荐使用以下这个函数添加，出错会自动清理
    udpevent* add_udpev(int port,const UCallback &rcb,const Callback &ecb);
    signalevent* add_sev(int signo,const SCallback& cb);
    signalevent* add_sev(const std::vector<int>& signals,const SCallback& cb);
    timerevent* add_timeev(int timeout_ms,bool periodic,const Callback &cb);
    
    /** v1.0.0 **/
    /* void add_ev(event *ev);
    void del_ev(event *ev);
    void mod_ev(event *ev);
    void add_bev(bfevent *bev);
    void del_bev(bfevent *bev);
    void mod_bev(bfevent *bev);
    void add_udpev(udpevent *uev);
    void del_udpev(udpevent *uev);
    void mod_udpev(udpevent *uev);
    void add_sev(signalevent* sev);
    void del_sev(signalevent* sev);
    void add_timeev(timerevent *tev);
    void del_timeev(timerevent *tev); */
private:
    void acceptcb_(int fd){
        bfevent *bev=new bfevent(pool_.ev_dispatch(),fd,EPOLLIN|EPOLLET);
        bev->setcb(readcb_,writecb_,std::bind(&server::tcp_eventcb_,this,bev));
        events_.emplace_back(bev);
    }

    void tcp_eventcb_(bfevent* bev){
        if(eventcb_) eventcb_();
        handle_close(bev);
    }

    void handle_close(base_event* ev){
        {
            std::lock_guard<std::mutex> lock(events_mutex_);
            events_.remove(ev);
        }
        ev->disable_cb();
        ev->getloop()->add_pending_del(ev);
    }
private:
    std::mutex events_mutex_;
    eventloop base_;    //主事件循环
    looptpool pool_;    //线程池，管理从reactor,分发事件
    acceptor acceptor_;     //tcp连接监听器
    int port_;      //tcp服务端端口号
    bool tcp_enable_=false;
    std::list<base_event*> events_;
    //tcp连接建立后设置事件的回调函数
    RCallback readcb_;   //会当读事件发生时触发
    Callback writecb_;  //这个类似于libevent库中一样，等bfevent自动将数据发送时才会触发
    Callback eventcb_;  //报错触发回调
};



}



#endif

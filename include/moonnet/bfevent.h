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

#ifndef _BFEVENT_H_
#define _BFEVENT_H_

#include "base_event.h"
#include "buffer.h"
#include "event.h"
#include <functional>
#include <unistd.h>

namespace moon {
class event;
class eventloop;

class bfevent:public base_event{
public:
    using RCallback=std::function<void(bfevent*)>;
    using Callback=std::function<void()>;
    bfevent(eventloop *base,int fd,uint32_t events);
    ~bfevent();
    int getfd() const;
    eventloop* getloop() const override;
    buffer* getinbuff();
    buffer* getoutbuff();
    bool writeable() const;
    void setcb(const RCallback &rcb,const Callback &wcb,const Callback &ecb);
    void setrcb(const RCallback &rcb);
    void setwcb(const Callback &wcb);
    void setecb(const Callback &ecb);
    RCallback getrcb();
    Callback getwcb();
    Callback getecb();
    void update_ep() override;   //更新监听事件
    void del_listen() override;  //取消监听
    void enable_listen() override;   //启动监听

    void sendout(const char* data,size_t len);
    void sendout(const std::string& data);
    size_t receive(char* data,size_t len);
    std::string receive(size_t len);
    std::string receive();
    void enable_events(uint32_t op);  //添加监听事件类型
    void disable_events(uint32_t op); //取消监听事件类型
    void enable_read();
    void disable_read();
    void enable_write();
    void disable_write();
    void enable_ET();
    void disable_ET();
    void disable_cb() override;

    void close() override{
        close_event();
    }
private:
    //关闭事件
    void close_event(){
        if(closed_) return;
        del_listen();
        ::close(fd_);
        closed_=true;
    }


    void handle_read(){
        while(true){
            int errnum=0;
            int n=inbuff_.readiov(fd_,errnum);
            if(n>0){
                if(readcb_) readcb_(this);
            }else if(n==0){
                if(eventcb_) eventcb_();
                break;
            }else{
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                else{
                    perror("read error");
                    if(eventcb_) eventcb_();
                    break;
                }
            }
        }
    }

    void handle_write(){
        while (outbuff_.readbytes()>0) {
            const char *data=outbuff_.peek();
            size_t len=outbuff_.readbytes();
            ssize_t n=write(fd_,data,len);
            if(n>0){
                outbuff_.retrieve(n);
                if(writecb_) writecb_();
            }else if(n==-1){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    break;
                }
                else{
                    perror("write error");
                    if(eventcb_) eventcb_();
                    break;
                }
            }
        }
        if(outbuff_.readbytes()==0) ev_->disable_write();
    }

    void handle_event(){
        if(eventcb_) eventcb_();
    }

private:
    eventloop *loop_;
    int fd_;
    event *ev_;
    buffer inbuff_;
    buffer outbuff_;
    RCallback readcb_;
    Callback writecb_;
    Callback eventcb_;
    bool closed_=false;
};


}

#endif

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


// udpevent.cpp
#include "udpevent.h"
#include "eventloop.h"
#include "event.h"
#include "wrap.h"
#include <cstring>

using namespace moon;

udpevent::udpevent(eventloop* base, int port)
    : loop_(base), fd_(-1), ev_(nullptr), receive_cb_(nullptr), started_(false)
{
    if(port!=-1) init_sock(port);
}

udpevent::~udpevent()
{
    if(ev_){
        delete ev_;
        ev_= nullptr;
    }
    if (fd_ != -1) {
        ::close(fd_);
        fd_=-1;
    }
}


void udpevent::init_sock(int port){
    // 创建 UDP 套接字
    fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_ < 0) {
        perror("UDP socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置套接字为非阻塞
    setnonblock(fd_);
    setreuse(fd_);
    // 绑定到指定端口
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(fd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("UDP bind failed");
        ::close(fd_);
        exit(EXIT_FAILURE);
    }
    ev_ = new event(loop_, fd_, EPOLLIN | EPOLLET);
    ev_->setcb(std::bind(&udpevent::handle_receive, this), nullptr, nullptr);
}


buffer* udpevent::getinbuff(){
    return &inbuff_;
}



eventloop* udpevent::getloop() const {
    return loop_;
}



void udpevent::setcb(const RCallback &rcb,const Callback &ecb){
    receive_cb_=rcb;
    event_cb_=ecb;
    update_ep();
}


void udpevent::setrcb(const udpevent::RCallback &rcb) {
    receive_cb_=rcb;
    update_ep();
}


void udpevent::setecb(const udpevent::Callback &ecb) {
    event_cb_=ecb;
    update_ep();
}


void udpevent::enable_listen()
{
    if (started_) return;
    if(ev_)
        ev_->enable_listen();
    started_ = true;
}


void udpevent::del_listen()
{
    if (!started_) return;
    if(ev_)
        ev_->del_listen();
    started_ = false;
}


void udpevent::update_ep(){
    if(started_)
        ev_->update_ep();
}


size_t udpevent::receive(char* data, size_t len){
    return inbuff_.remove(data, len);
}


std::string udpevent::receive(size_t len){
    return inbuff_.remove(len);
}


std::string udpevent::receive(){
    return inbuff_.remove(inbuff_.readbytes());
}


void udpevent::send_to(const std::string& data, const sockaddr_in& addr)
{
    ssize_t n = sendto(fd_, data.c_str(), data.size(), 0, (struct sockaddr*)&addr, sizeof(addr));
    if (n < 0) {
        perror("sendto failed");
    }
}

void udpevent::handle_receive()
{
    while (true) {
        char buf[BUFSIZE];
        struct sockaddr_in cli_addr;
        socklen_t cli_len = sizeof(cli_addr);
        ssize_t n = recvfrom(fd_, buf, sizeof(buf), 0, (struct sockaddr*)&cli_addr, &cli_len);
        if (n > 0) {
            inbuff_.append(buf,n);
            if (receive_cb_) {
                receive_cb_(cli_addr,this);
            }
        }
        else if (n == 0) {
            // No data
            if(event_cb_) event_cb_();
            break;
        }
        else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // 没有更多数据
            }
            else {
                perror("recvfrom failed");
                if(event_cb_) event_cb_();
                break;
            }
        }
    }
}


void udpevent::enable_read(){
    ev_->enable_read();
}


void udpevent::enable_ET(){
    ev_->enable_ET();
}


void udpevent::disable_read(){
    ev_->disable_read();
}


void udpevent::disable_ET(){
    ev_->disable_read();
}


void udpevent::disable_cb() {
    receive_cb_= nullptr;
    event_cb_= nullptr;
}


udpevent::RCallback udpevent::getrcb() {
    return receive_cb_;
}


udpevent::Callback udpevent::getecb() {
    return event_cb_;
}

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


#include "acceptor.h"
#include "event.h"
#include "eventloop.h"
#include "wrap.h"

using namespace moon;

acceptor::acceptor(int port,eventloop *base)
:loop_(base),shutdown_(true){
    if(port!=-1)
        init_sock(port);
}


acceptor::~acceptor(){
    stop();
    delete ev_;
    close(lfd_);
}




void acceptor::init_sock(int port){
    int fd=Socket(AF_INET,SOCK_STREAM,0);
    setnonblock(fd);
    setreuse(fd);

    struct sockaddr_in ser_addr;
    bzero(&ser_addr,sizeof(ser_addr));
    ser_addr.sin_family=AF_INET;
    ser_addr.sin_addr.s_addr=INADDR_ANY;
    ser_addr.sin_port=htons(port);
    
    Bind(fd,(struct sockaddr*)&ser_addr,sizeof(ser_addr));
    Listen(fd,SOMAXCONN);
    
    lfd_=fd;
}



void acceptor::setcb(const Callback &accept_cb){
    cb_=accept_cb;
}


void acceptor::handle_accept(){
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    bzero(&cli_addr,cli_len);
    while(true){
        int cfd=accept(lfd_,(struct sockaddr*)&cli_addr,&cli_len);
        if (cfd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // 没有更多的连接
            } else {
                perror("accept");
                break;
            }
        }
        setnonblock(cfd);
        settcpnodelay(cfd);
        if(cb_) cb_(cfd);
    }

}


void acceptor::listen(){
    if(!shutdown_) return;
    ev_=new event(loop_,lfd_,EPOLLIN|EPOLLET);
    ev_->setcb(std::bind(&acceptor::handle_accept,this),NULL,NULL);
    loop_->add_event(ev_);
    shutdown_=false;
}


void acceptor::stop(){
    if(shutdown_) return;
    ev_->del_listen();
    shutdown_=true;
}

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


#include "bfevent.h"
#include <cstdio>
#include "event.h"
#include "eventloop.h"

using namespace moon;

bfevent::bfevent(eventloop *base,int fd,uint32_t events)
:loop_(base),fd_(fd),ev_(new event(base,fd,events)){
    ev_->setcb(std::bind(&bfevent::handle_read,this),
              std::bind(&bfevent::handle_write,this),
              std::bind(&bfevent::handle_event,this));
    ev_->enable_listen();
}


bfevent::~bfevent(){
    close_event();
    delete ev_;
}


int bfevent::getfd() const{
    return fd_;
}


eventloop* bfevent::getloop() const{
    return loop_;
}



buffer* bfevent::getinbuff(){
    return &inbuff_;
}


buffer* bfevent::getoutbuff(){
    return &outbuff_;
}


bool bfevent::writeable() const{
    return ev_->writeable();
}


void bfevent::setcb(const RCallback &rcb,const Callback &wcb,const Callback &ecb){
    readcb_=rcb;
    writecb_=wcb;
    eventcb_=ecb;
    update_ep();
}



void bfevent::setrcb(const RCallback &rcb){
    readcb_=rcb;
    update_ep();
}



void bfevent::setwcb(const Callback &wcb){
    writecb_=wcb;
    update_ep();
}



void bfevent::setecb(const Callback &ecb){
    eventcb_=ecb;
    update_ep();
}



void bfevent::enable_events(uint32_t op){
    ev_->enable_events(op);
}


void bfevent::disable_events(uint32_t op){
    ev_->disable_events(op);
}


void bfevent::update_ep(){
    if(!closed_)
        ev_->update_ep();
}


void bfevent::del_listen(){
    if(inbuff_.readbytes()>0){
        if(readcb_) readcb_(this);
    }
    if(outbuff_.readbytes()>0){
        if(!ev_->writeable()) ev_->enable_write();
    }
    if(outbuff_.readbytes()==0) ev_->disable_write();
    ev_->del_listen();
    closed_=true;
}


void bfevent::enable_listen(){
    if(!closed_) return;
    ev_->enable_listen();
    closed_=false;
}



void bfevent::sendout(const char* data, size_t len){
    ssize_t n=0;
    ssize_t relen=len;
    if(!writeable()&&outbuff_.readbytes()==0){
        n=write(fd_,data,len);
        if(n>=0){
            relen-=len;
            if(relen==0&&writecb_){
                writecb_();
                return;
            }
        }else{
            if (errno != EAGAIN && errno != EWOULDBLOCK){
                perror("sendout error");
                return;
            }
        }
    }

    if(relen>0){
        struct iovec vec[2];
        size_t wbytes=outbuff_.readbytes();
        vec[0].iov_base=const_cast<char*>(outbuff_.peek());
        vec[0].iov_len=wbytes;
        vec[1].iov_base=const_cast<char*>(data+n);
        vec[1].iov_len=relen;
        ssize_t wvn=writev(fd_,vec,2);
        if(wvn>=0){
            size_t tlen=wbytes+relen;
            if(wvn<tlen){
                if(wvn<wbytes) outbuff_.retrieve(wvn);
                else{
                    outbuff_.reset();
                    size_t diff=wvn-wbytes;
                    relen-=diff;
                    data+=(n+diff);
                }
            }
            else{
                outbuff_.reset();
                if(writecb_) writecb_();
                return;
            }
        }else{
            if (errno != EAGAIN && errno != EWOULDBLOCK){
                perror("sendout error");
                return;
            }
        }
    }
    if(relen>0){
        outbuff_.append(data, relen);
        if(!writeable())
            ev_->enable_write();
    }
}

void bfevent::sendout(const std::string& data){
    sendout(data.c_str(), data.size());
}

size_t bfevent::receive(char* data, size_t len){
    return inbuff_.remove(data, len);
}

std::string bfevent::receive(size_t len){
    return inbuff_.remove(len);
}


std::string bfevent::receive(){
    return inbuff_.remove(inbuff_.readbytes());
}


void bfevent::enable_read(){
    ev_->enable_read();
}


void bfevent::enable_write(){
    ev_->enable_write();
}


void bfevent::enable_ET(){
    ev_->enable_ET();
}


void bfevent::disable_read(){
    ev_->disable_read();
}


void bfevent::disable_write(){
    ev_->disable_write();
}


void bfevent::disable_ET(){
    ev_->disable_ET();
}


void bfevent::disable_cb() {
    readcb_= nullptr;
    writecb_= nullptr;
    eventcb_= nullptr;
}


bfevent::RCallback bfevent::getrcb() {
    return readcb_;
}


bfevent::Callback bfevent::getwcb() {
    return writecb_;
}


bfevent::Callback bfevent::getecb() {
    return eventcb_;
}

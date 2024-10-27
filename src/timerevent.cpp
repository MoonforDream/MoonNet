#include <sys/timerfd.h>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include "eventloop.h"
#include "event.h"
#include "timerevent.h"


using namespace moon;

timerevent::timerevent(eventloop *loop, int timeout_ms, bool periodic)
:loop_(loop),timeout_ms_(timeout_ms),periodic_(periodic){
    _init_();
}



timerevent::~timerevent(){
    if(ev_){
        delete ev_;
        ev_= nullptr;
    }
    if(fd_!=-1){
        ::close(fd_);
        fd_=-1;
    }
}

void timerevent::_init_() {
    fd_=timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);
    if(fd_==-1){
        perror("timerfd create");
        exit(EXIT_FAILURE);
    }

    itimerspec tvalue;
    //it_value设置定时器第一次到期时间
    tvalue.it_value.tv_sec=timeout_ms_/1000; //初始化到期秒数
    tvalue.it_value.tv_nsec=(timeout_ms_%1000)*1000000;  //初始化到期纳秒数
    tvalue.it_interval={0,0};
    //it_interval设置定时器第一次之后每次到期的间隔时间，设置为0,定时器只会触发一次,非0为周期性触发
    if(periodic_){
        tvalue.it_interval.tv_sec=timeout_ms_/1000;  //间隔时间秒数
        tvalue.it_interval.tv_nsec=(timeout_ms_%1000)*1000000;   //间隔时间的纳秒数
    }

    if(timerfd_settime(fd_,0,&tvalue,NULL)==-1){
        perror("timerfd_settime error");
        ::close(fd_);
        exit(EXIT_FAILURE);
    }
    ev_=new event(loop_,fd_,EPOLLIN);
    ev_->setcb(std::bind(&timerevent::handle_timeout,this), nullptr, nullptr);
}


int timerevent::getfd() const {
    return fd_;
}


eventloop* timerevent::getloop() const {
    return loop_;
}


void timerevent::setcb(const moon::timerevent::Callback &cb) {
    cb_=cb;
}


void timerevent::start() {
    if(ev_){
        ev_->enable_listen();
    }
}


void timerevent::stop() {
    if(ev_)
        ev_->del_listen();
}


void timerevent::handle_timeout() {
    uint64_t exp;
    ssize_t s=read(fd_,&exp,sizeof(exp));
    if(s!= sizeof(exp)){
        perror("read timerfd");
        return;
    }
    if(cb_) cb_();
}


void timerevent::disable_cb() {
    cb_= nullptr;
}


timerevent::Callback timerevent::getcb() {
    return cb_;
}
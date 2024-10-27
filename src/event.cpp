#include "event.h"
#include "eventloop.h"



using namespace moon;

event::event(eventloop *base,int fd,uint32_t events)
        :loop_(base),fd_(fd),events_(events){
}

event::~event(){
}

int event::getfd() const{
    return fd_;
}

uint32_t event::getevents() const{
    return events_;
}


eventloop* event::getloop() const{
    return loop_;
}


void event::setcb(const Callback &rcb,const Callback &wcb,const Callback &ecb){
    readcb_=rcb;
    writecb_=wcb;
    eventcb_=ecb;
}



void event::setrcb(const Callback &rcb){
    readcb_=rcb;
}


void event::setwcb(const Callback &wcb){
    writecb_=wcb;
}


void event::setecb(const Callback &ecb){
    eventcb_=ecb;
}

void event::setrevents(const uint32_t revents){
    revents_=revents;
}

void event::update_ep(){
    loop_->mod_event(this);
}

void event::enable_events(uint32_t op){
    events_|=op;
    update_ep();
}

void event::disable_events(uint32_t op){
    events_&=~op;
    update_ep();
}



bool event::readable(){
    return (events_&EPOLLIN);
}


bool event::writeable(){
    return (events_&EPOLLOUT);
}


void event::enable_read(){
    enable_events(EPOLLIN);
}


void event::disable_read(){
    disable_events(EPOLLIN);
}


void event::enable_write(){
    enable_events(EPOLLOUT);
}


void event::disable_write(){
    disable_events(EPOLLOUT);
}


void event::enable_ET(){
    enable_events(EPOLLET);
}


void event::disable_ET(){
    disable_events(EPOLLET);
}


void event::reset_events(){
    events_=0;
    update_ep();
}


void event::del_listen(){
    loop_->del_event(this);
}


void event::enable_listen(){
    loop_->add_event(this);
}



void event::disable_cb() {
    readcb_= nullptr;
    writecb_= nullptr;
    eventcb_= nullptr;
}

void event::handle_cb(){
    if((revents_ & EPOLLIN) || (revents_ & EPOLLRDHUP) || (revents_ & EPOLLPRI))
    {
        if(readcb_) readcb_();
    }
    if(revents_&EPOLLOUT){
        if(writecb_) writecb_();
    }
    if(revents_ & (EPOLLERR|EPOLLHUP)){
        if(eventcb_) eventcb_();
    }
}



event::Callback event::getrcb() {
    return readcb_;
}


event::Callback event::getwcb() {
    return writecb_;
}


event::Callback event::getecb() {
    return eventcb_;
}
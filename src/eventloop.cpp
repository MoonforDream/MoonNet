#include "base_event.h"
#include "eventloop.h"
#include "event.h"

using namespace moon;

eventloop::eventloop(loopthread* base,int timeout)
:baseloop_(base),timeout_(timeout),shutdown_(false),load_(0),events_(MAX_EVENTS){
    epfd_=epoll_create1(0);
    if(-1==epfd_){
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    create_eventfd();
}


eventloop::~eventloop(){
    loopbreak();
    for(auto &ev : evlist_){
        delete ev;
    }
    for(auto &ev:delque_){
        delete ev;
    }
    evlist_.clear();
    delque_.clear();
    close(epfd_);
    close(eventfd_);
}



loopthread* eventloop::getbaseloop(){
    if(!baseloop_) return nullptr;
    return baseloop_;
}


int eventloop::getefd() const{
    return epfd_;
}


int eventloop::getevfd() const{
    return eventfd_;
}


int eventloop::getload() const{
    return load_;
}


void eventloop::add_event(event* event){
    int fd=event->getfd();
    struct epoll_event ev;
    ev.data.ptr=event;
    //ev.data.fd=fd;
    ev.events=event->getevents();

    if(epoll_ctl(epfd_,EPOLL_CTL_ADD,fd,&ev)==-1){
        perror("epoll_ctl add error");
    }
    updateload(1);
    evlist_.emplace_back(event);
}


void eventloop::del_event(event* event){
    int fd=event->getfd();

    if(epoll_ctl(epfd_,EPOLL_CTL_DEL,fd,nullptr)==-1){
        perror("epoll_ctl del error");
    }
    updateload(-1);
    evlist_.remove(event);
}


void eventloop::mod_event(event* event){
    int fd=event->getfd();
    struct epoll_event ev;
    ev.data.ptr=event;
    ev.events=event->getevents();

    if(epoll_ctl(epfd_,EPOLL_CTL_MOD,fd,&ev)==-1){
        perror("epoll_ctl modify error");
    }
}



void eventloop::loop(){
    while (!shutdown_) {
        int n=epoll_wait(epfd_,events_.data(),MAX_EVENTS,timeout_);
        if(-1==n){
            if(errno==EINTR) continue;
            perror("epoll_wait");
            break;
        }
        for(int i=0;i<n;++i){
            auto ev=static_cast<event*>(events_[i].data.ptr);
            ev->setrevents(events_[i].events);
            ev->handle_cb();
        }
        if(n==events_.size()){
            events_.resize(events_.size() * 2);
        }
        if(!delque_.empty()){
            for(auto ev : delque_){
                delete ev;
            }
            delque_.clear();
        }
    }
}


void eventloop::loopbreak(){
    if(shutdown_) return;
    write_eventfd();
}


void eventloop::getallev(std::list<event*> &list){
    list.swap(evlist_);
}


void eventloop::create_eventfd(){
    eventfd_=eventfd(0,EFD_CLOEXEC | EFD_NONBLOCK);
    if(eventfd_<0){
        perror("eventfd create error");
        exit(EXIT_FAILURE);
    }
    event *ev=new event(this,eventfd_,EPOLLIN);
    ev->setcb(std::bind(&eventloop::read_eventfd,this),NULL,NULL);
    add_event(ev);
}


void eventloop::read_eventfd(){
    uint64_t opt=1;
    ssize_t n=read(eventfd_,&opt,sizeof(opt));
    shutdown_=true;
    if(n<0){
        if(errno==EINTR || errno == EAGAIN){
            return;
        }
        perror("eventfd read error");
        exit(EXIT_FAILURE);
    }
}



void eventloop::write_eventfd(){
    uint64_t opt=1;
    ssize_t n=write(eventfd_,&opt,sizeof(opt));
    if(n<0){
        if(errno==EINTR){
            return;
        }
        perror("eventfd write error");
        exit(EXIT_FAILURE);
    }
}


void eventloop::add_pending_del(base_event *ev) {
    delque_.emplace_back(ev);
}




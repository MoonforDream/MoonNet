#ifndef _EVENTLOOP_H_
#define _EVENTLOOP_H_


#include "wrap.h"
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <functional>
#include <vector>
#include <list>
#include <unistd.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>


#define MAX_EVENTS 65536


namespace moon {

class base_event;
class event;
class loopthread;

//reactor类-->事件循环类
class eventloop{
public:
    using Callback=std::function<void()>;
    eventloop(loopthread* base=nullptr,int timeout=-1);
    ~eventloop();
    loopthread* getbaseloop();
    int getefd() const;
    int getevfd() const;
    int getload() const;
    //事件控制函数
    void add_event(event* event);
    void del_event(event* event);
    void mod_event(event* event);

    // void loop(struct timeval *tv);
    void loop();
    void loopbreak(); 
    void getallev(std::list<event*> &list);

    void create_eventfd();     //创建通知文件描述符
    void read_eventfd();
    void write_eventfd();

    void add_pending_del(base_event* ev);
private:
    //更新负载
    void updateload(int n){
        load_+=n;
    }


private:
    int epfd_;
    int eventfd_;
    int timeout_=-1;
    std::atomic<int> load_;
    std::atomic<bool> shutdown_;
    std::list<event*> evlist_;
    std::vector<epoll_event> events_;
    std::vector<base_event*> delque_;
    loopthread *baseloop_;
};
}


#endif 

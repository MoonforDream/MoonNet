// Modified from muduo project http://github.com/chenshuo/muduo
// @see https://github.com/chenshuo/muduo/blob/master/muduo/net/EventLoopThread.h
// and https://github.com/chenshuo/muduo/blob/master/muduo/net/EventLoopThread.cc


#ifndef _LOOPTHREAD_H_
#define _LOOPTHREAD_H_


#include <condition_variable>
#include <mutex>
#include <thread>



#define MAX_EPOLL_TIMEOUT_MSEC (35*60*1000)

namespace moon {

class eventloop;

class loopthread{
public:
    loopthread(int timeout=-1)
    :loop_(nullptr),t_(std::thread(&loopthread::_init_,this)),timeout_(timeout){
        if(timeout_>MAX_EPOLL_TIMEOUT_MSEC){
            timeout_=MAX_EPOLL_TIMEOUT_MSEC;
        }
    }
    ~loopthread();
    void _init_();  //初始化
    eventloop* getloop();   //获取loop_
    void join(){
        if(t_.joinable()) t_.join();
    }
private:
    eventloop *loop_;
    std::mutex mx_;
    std::condition_variable cv_;
    std::thread t_;
    int timeout_=-1;     //设置epoll间隔检测时间ms
};

}


#endif // !_LOOPTHREAD_H_

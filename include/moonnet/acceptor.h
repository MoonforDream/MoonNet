#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include <functional>


namespace moon {

class eventloop;
class event;

//监听连接类
class acceptor{
public:
    using Callback=std::function<void(int)>;
    acceptor(int port,eventloop *base);
    ~acceptor();
    void listen();  //开始监听
    void stop();    //停止监听
    void init_sock(int port);    //建立监听套接字
    void setcb(const Callback &accept_cb); //设置回调函数
    void handle_accept();       //acceptor事件回调函数，用来接收连接
private:
    int lfd_;
    eventloop *loop_;
    event *ev_;
    Callback cb_;   //连接后回调函数
    bool shutdown_;
};

}

#endif


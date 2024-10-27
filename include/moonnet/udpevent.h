// udpevent.h
#ifndef _UDPEVENT_H_
#define _UDPEVENT_H_

#include "base_event.h"
#include "buffer.h"
#include <functional>
#include <unistd.h>
#include <arpa/inet.h>

namespace moon {

class eventloop;
class event;

// UDP 处理类
class udpevent :public base_event{
public:
    using Callback = std::function<void()>;
    using RCallback=std::function<void(const sockaddr_in&,udpevent*)>;
    udpevent(eventloop* base,int port);
    ~udpevent();
    
    buffer* getinbuff();
    eventloop* getloop() const override;
    void setcb(const RCallback &rcb,const Callback &ecb);
    void setrcb(const RCallback &rcb);
    void setecb(const Callback &ecb);
    void init_sock(int port);
    void start();   // 开始监听
    void stop();    // 停止监听
    void update_ep();   //更新监听事件
    size_t receive(char* data,size_t len);
    std::string receive(size_t len);
    std::string receive();
    void send_to(const std::string& data, const sockaddr_in& addr); // 发送数据到指定地址
    
    void enable_read();
    void disable_read();
    void enable_ET();
    void disable_ET();
    void disable_cb() override;
    RCallback getrcb();
    Callback getecb();
    void close() override{
        stop();
    }
private:
    void handle_receive(); // 处理接收事件

private:
    eventloop* loop_;
    int fd_;
    event *ev_;
    buffer inbuff_;
    RCallback receive_cb_;
    Callback event_cb_;
    bool started_=false;
};

} // namespace moon

#endif

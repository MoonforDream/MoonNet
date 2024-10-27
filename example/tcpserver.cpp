#include "moonnet.h"
#include <signal.h>
#include <iostream>

using namespace moon;


// signalevent callback / 信号事件回调函数
void handle_signal(int signo, server *ser){
    std::cout << "Caught signal " << signo << std::endl;
    // Close the server and clean up resources / 关闭服务器，清理资源
    ser->stop();
    std::cout << "TCP Echo Server Termination" << std::endl;
}

void handle_read(bfevent* bev){
    std::string str = bev->receive();
    // Or get inbuff / 或者获取输入缓冲区
    /* buffer* buff = bev->getinbuff();
    std::string s = bev->receive(buff->readbytes()); */
    bev->sendout(str);
}


void handle_write(){
    std::cout << "send success" << std::endl;
}

// UDP receive callback function / UDP 接收回调函数
void on_udp_receive(const sockaddr_in& addr, moon::udpevent* uev) {
    std::string data = uev->receive();
    if (!data.empty()) {
        std::cout << "UDP Received from " << inet_ntoa(addr.sin_addr)
                  << ":" << ntohs(addr.sin_port) << " - " << data << std::endl;

        // Send reply / 发送回复
        std::string reply = "Echo: " + data;
        std::cout << reply << std::endl;
        uev->send_to(reply, addr);
    }
}

// UDP other event callback function (e.g., error) / UDP 其他事件回调函数（如错误）
void on_udp_event() {
    std::cout << "An error occurred on the UDP connection." << std::endl;
}

// Server class constructor has set the connection callback in acceptcb_
// 服务器类构造函数已在 acceptcb_ 中设置了连接回调
int main() {
    // Initialize server, listen on port 5005 / 初始化服务器，监听端口 5005
    server srv;
    // Enable TCP service / 启用 TCP 服务
    srv.enable_tcp(5005);

    // Initialize thread pool and start dynamic balancing algorithm by default / 初始化线程池，默认启动动态均衡算法
    srv.init_pool();
    // Initialize thread pool with specified number of threads and enable dynamic balancing algorithm / 以指定线程数初始化线程池,并启用动态均衡算法
    //or
    // srv.init_pool(4,-1);
    // or 
    // Initialize thread pool without using dynamic load balancing algorithm / 不使用动态负载均衡算法初始化线程池
    // srv.init_pool_noadjust(4,-1);

    // Set TCP connection callbacks / 设置 TCP 连接的回调函数
    srv.set_tcpcb(handle_read, handle_write, nullptr);

    // Add signalevent / 添加信号事件
    std::vector<int> v = {SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM};
    srv.add_sev(v, std::bind(handle_signal, std::placeholders::_1, &srv));
    // Or add signalevent / 或添加信号事件
//    signalevent sev(srv.getloop());
//    sev.add_signal(v);
//    sev.setcb(std::bind(handle_signal, std::placeholders::_1, &srv));
//    sev.enable_listen();


    // Add udpevent / 添加udp事件
    srv.add_udpev(5006, on_udp_receive, on_udp_event);

    // Start server / 启动服务器
    srv.start();

    return 0;
}

#include "moonnet.h"
#include <iostream>

// UDP 接收回调函数
void on_udp_receive(const sockaddr_in& addr, moon::udpevent* uev) {
    std::string data = uev->receive();
    if (!data.empty()) {
        std::cout << "UDP Received from " << inet_ntoa(addr.sin_addr)
                  << ":" << ntohs(addr.sin_port) << " - " << data << std::endl;

        // 发送回复
        std::string reply = "Echo: " + data;
        std::cout<<reply<<std::endl;
        uev->send_to(reply, addr);
    }
}

// UDP 其他事件回调函数（如错误）
void on_udp_event() {
    std::cout << "An error occurred on the UDP connection." << std::endl;
}

int main() {
    // 创建服务器，不指定 TCP 端口
    moon::server udp_server(-1);
//    udp_server.init_pool();
    // 添加 UDP 事件，监听端口 5005
    moon::udpevent* uev = udp_server.add_udpev(5005,on_udp_receive, on_udp_event);

    // 启动服务器
    udp_server.start();

    return 0;
}

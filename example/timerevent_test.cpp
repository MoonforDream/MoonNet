#include "moonnet.h"
#include <iostream>

// 定时器回调函数
void on_timer() {
    std::cout << "Timer triggered!" << std::endl;
}

int main() {
    // 创建服务器，不指定 TCP 端口
    moon::server timer_server(-1);

    // 添加一个周期性定时器，每 1000 毫秒触发一次
    moon::timerevent* tev = timer_server.add_timeev(1000, true, on_timer);

    // 启动服务器
    timer_server.start();

    return 0;
}

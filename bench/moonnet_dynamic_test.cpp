#include <moonnet/moonnet.h>
#include <iostream>

using namespace moon;



void handle_read(bfevent* bev){
    std::string str = bev->receive();
    bev->sendout(str);
}


int main() {
    server srv;
    srv.enable_tcp(5005);
    srv.init_pool(2,-1); //2-3线程，bench测试时更改了源码的settnum,设置成2-4线程
    srv.set_tcpcb(handle_read, nullptr, nullptr);
    srv.start();

    return 0;
}

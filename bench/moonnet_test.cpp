#include "MoonNet.h"
#include <iostream>

using namespace moon;



void handle_read(bfevent* bev){
    std::string str = bev->receive();
    bev->sendout(str);
}


int main() {
    server srv;
    srv.enable_tcp(5005);
    srv.init_pool_noadjust(4,-1);
    srv.set_tcpcb(handle_read, nullptr, nullptr);
    srv.start();

    return 0;
}

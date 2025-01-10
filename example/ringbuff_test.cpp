#include <moonnet/moonnet.h> //可以只引用总头文件/You can only reference the header file
//#include <moonnet/ringbuff.h>
#include <iostream>
#include <string>
#include <vector>

int main() {
    moon::ringbuff<std::string> buffer(32); // 初始化一个环形缓冲区，容量调整为最近的二次幂

    // 向环形缓冲区中推送元素，使用 push 和 push_move
    buffer.push("你好");
    buffer.push_move(std::string("世界"));

    // 从环形缓冲区中弹出元素
    std::string data;
    if (buffer.pop(data)) {
        std::cout << "弹出: " << data << std::endl;
    }

    // 检查缓冲区大小和容量
    std::cout << "当前缓冲区大小: " << buffer.size() << std::endl;
    std::cout << "缓冲区容量: " << buffer.capacity() << std::endl;

    // 检查缓冲区是否为空或已满
    std::cout << "缓冲区是否为空? " << (buffer.empty() ? "是" : "否") << std::endl;
    std::cout << "缓冲区是否已满? " << (buffer.full() ? "是" : "否") << std::endl;

    // 使用 swap_to_vector 导出缓冲区内容
    std::vector<std::string> vec;
    buffer.swap_to_vector(vec);

    // 显示导出的数据
    for (auto &str : vec) {
        std::cout << "导出: " << str << std::endl;
    }

    return 0;
}

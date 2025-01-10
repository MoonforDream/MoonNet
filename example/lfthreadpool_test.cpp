#include <moonnet/moonnet.h> //可以只引用总头文件/You can only reference the header file
//#include <moonnet/lfthreadpool.h>
#include <iostream>
#include <functional>

void printTask(int num) {
    std::cout << "任务 " << num << " 执行完毕。" << std::endl;
}

int main() {
    // tnum为-1则表示采用系统自动设置线程池大小
//    moon::lfthreadpool pool(4, 1024, moon::PoolMode::Dynamic); // 使用动态模式初始化线程池
    // 采用系统设置线程池大小，以及使用静态模式
    moon::lfthreadpool pool(-1,32);

    // 向池中添加任务，使用 add_task 和 add_task_move
    for (int i = 0; i < 10; i++) {
        pool.add_task(printTask, i);
        pool.add_task_move([=] { printTask(i + 10); });
    }

    // 留出一些时间让任务执行
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 关闭线程池
    pool.t_shutdown();

    return 0;
}

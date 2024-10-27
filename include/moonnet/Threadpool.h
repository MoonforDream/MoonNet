#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#define DEFAULT_TIME 10

#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <vector>
#include <mutex>
#include <utility>
#include <atomic>


namespace moon{

class Threadpool{
public:
    Threadpool(int num):threads(std::vector<std::thread>(num)),min_thr_num(num),live_num(num){
        init();
    }
    ~Threadpool(){
        t_shutdown();
    }
    //初始化线程池
    void init();
    //销毁线程池并释放资源
    void t_shutdown();
    //各任务线程入口函数
    void t_task();
    //管理线程入口函数
    void adjust_task();
    //添加任务
    template<typename _Fn,typename... _Args>
    void add_task(_Fn&& fn,_Args&&... args){
        {
            auto f=std::bind(std::forward<_Fn>(fn),std::forward<_Args>(args)...);
            {
                std::unique_lock<std::mutex> lock(mx);
                if(shutdown) return;
                tasks.emplace(std::move(f));
            }
            task_cv.notify_one();
        }
    }
private:
    std::thread adjust_thr;     //管理线程
    std::vector<std::thread> threads;   //线程数组
    std::queue<std::function<void()>> tasks;    //任务队列
    std::mutex mx;  //线程池锁
    std::condition_variable task_cv;    //任务通知条件变量
    int min_thr_num=0;    //线程池最小线程数
    int max_thr_num=0;     //cpu核数的2n+1
    std::atomic<int> run_num; //线程池中正在执行任务的线程数
    std::atomic<int> live_num; //线程池空闲线程数
    std::atomic<int> exit_num; //线程池要销毁线程数
    bool shutdown=false;  //线程池状态，true为运行，false为关闭
};


}

#endif

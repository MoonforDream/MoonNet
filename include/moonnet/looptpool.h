#ifndef _LOOPTPOOL_H_
#define _LOOPTPOOL_H_



#include <thread>
#include <vector>
#include <set>

#define ADJUST_TIMEOUT_SEC 5


namespace moon {

class eventloop;
class loopthread;


class looptpool{
public:
    looptpool(eventloop *base,bool dispath=false);      //默认不开启动态负载均衡
    ~looptpool();
    void create_pool(int timeout=-1);     //创建线程池
    void create_pool(int n,int timeout);     //可以指定线程的线程池
    void create_pool_noadjust(int n,int timeout);   //不进行调度管理的指定线程初始化线程池
    eventloop* ev_dispatch();   //分发事件
    void delloop_dispatch();    //删除从reactor并分发事件
    void addloop();     //添加eventloop(从reactor)
    void adjust_task();     //管理线程任务，调度管理从reactor
    int getscale();     //获取平均负载
    void enable_adjust();   //启用动态均衡调度任务
    looptpool(const looptpool&)=delete;
    looptpool& operator=(const looptpool&)=delete;
    void stop();    //终止运行
private:
    void init_pool(int timeout=-1);
    unsigned int getcore(){
        unsigned int cpu_cores = std::thread::hardware_concurrency()/2;
        if (cpu_cores == 0) {
            cpu_cores = 4; // 默认值
        }
        return cpu_cores+1;
    }

    void settnum(){
        t_num=getcore();
        max_tnum=t_num*2-1;
        min_tnum=t_num;
    }

    void settnum(int n){
        t_num=n;
        max_tnum=t_num*2-1;
        min_tnum=t_num;
    }

    void settnum_noadjust(int n){
        t_num=n;
        min_tnum=n;
        max_tnum=n;
    }

    void init_adjust(){
        manager_=(std::thread(&looptpool::adjust_task,this));
    }

    //终止调度任务,避免多次取消管理任务和添加管理任务
    void stop_adjust(){
        if(!dispath_) return;
        dispath_=false;
    }

    eventloop* getminload();
    int getmaxidx();
private:
    eventloop *baseloop_;
    std::thread manager_;
    std::vector<eventloop*> loadvec_;
    int next_=0;
    int t_num=0;
    int timeout_=-1;
    int max_tnum=0;
    int min_tnum=0;
    bool dispath_;
    int coolsec_=30;
    int timesec_=5;
    int scale_max=80;
    int scale_min=20;
};

}




#endif
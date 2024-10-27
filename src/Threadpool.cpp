#include "Threadpool.h"
#include <chrono>
#include <mutex>
#include <unistd.h>


using namespace moon;

void Threadpool::init(){
    run_num=0;
    live_num=0;
    exit_num=0;
    adjust_thr=std::thread([this]{
        this->adjust_task();
    });
    for(int i=0;i<min_thr_num;++i){
        threads.emplace_back([this]{
            this->t_task();
        });
    }
}




void Threadpool::t_task(){
    while (1) {
        std::unique_lock<std::mutex> lock(mx);
        task_cv.wait(lock,[this]{
            return !tasks.empty()||shutdown||exit_num>0;
        });
        if(exit_num>0){
            exit_num--;
            return;
        }
        if(shutdown&&tasks.empty()){
            return;
        }
        auto task=tasks.front();
        tasks.pop();
        lock.unlock();
        ++run_num;
        --live_num;
        task();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


void Threadpool::t_shutdown(){
    {
        std::unique_lock<std::mutex> lock(mx);
        shutdown=true;
    }
    adjust_thr.detach();
    task_cv.notify_all();
    for(auto& t:threads){
        if(t.joinable()) t.join();
    }
}



void Threadpool::adjust_task(){
    while (!shutdown) {
        std::this_thread::sleep_for(std::chrono::seconds(DEFAULT_TIME));
        {
            int size=threads.size();
            if (tasks.size() > live_num && live_num < max_thr_num&&size<max_thr_num) {
                int add = 0;
                std::unique_lock<std::mutex> lock(mx);
                for (int i = size; i < max_thr_num && add < 10; ++i) {
                    threads.emplace_back([this] {
                        this->t_task();
                    });
                    add++;
                    live_num++;
                }
                lock.unlock();
            }
            if (run_num * 2 < live_num && live_num > min_thr_num) {
                exit_num=live_num-min_thr_num>=10?10:live_num-min_thr_num;
                int x=exit_num;
                std::unique_lock<std::mutex> lock(mx);
                for (int i = 0; i < x; ++i) {
                    task_cv.notify_one();
                    live_num--;
                }
                lock.unlock();
            }
        }
    }
}



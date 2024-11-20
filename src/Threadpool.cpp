/* BSD 3-Clause License

Copyright (c) 2024, MoonforDream

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

Author: MoonforDream

*/


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



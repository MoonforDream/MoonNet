#include "looptpool.h"
#include "eventloop.h"
#include "loopthread.h"

using namespace moon;


looptpool::looptpool(eventloop *base,bool dispath)
:baseloop_(base),dispath_(dispath){
    if(dispath_){
        init_adjust();
    }
};




looptpool::~looptpool(){
   stop();
    for(auto &t:loadvec_){
        delete t->getbaseloop();
    }
    loadvec_.clear();
    t_num=0;
}



void looptpool::init_pool(int timeout){
    timeout_=timeout;
    for(int i=0;i<t_num;++i){
        loopthread* lt=new loopthread(timeout);
        loadvec_.emplace_back(lt->getloop());
    }
}


void looptpool::create_pool(int timeout){
    dispath_=true;
    settnum();
    init_pool(timeout);
    init_adjust();
}

void looptpool::create_pool(int n,int timeout){
    dispath_=true;
    settnum(n);
    init_pool(timeout);
    init_adjust();
}



void looptpool::create_pool_noadjust(int n,int timeout){
    settnum_noadjust(n);
    init_pool(timeout);
    stop_adjust();
}


eventloop* looptpool::ev_dispatch(){
    if(t_num==0) return baseloop_;
    if(dispath_){
        return getminload();
    }else{
        next_=(next_+1)%t_num;
        return loadvec_[next_];
    }
}


void looptpool::delloop_dispatch(){
    int idx=getmaxidx();
    eventloop* ep=loadvec_[idx];
    std::list<event*> list;
    ep->getallev(list);
    for(auto &ev:list){
        ev_dispatch()->add_event(ev);
    }
    loadvec_[idx]=std::move(loadvec_.back());
    loadvec_.pop_back();
    delete ep->getbaseloop();
    --t_num;
}


void looptpool::addloop(){
    loopthread* lt=new loopthread(timeout_);
    loadvec_.emplace_back(lt->getloop());
    ++t_num;
}



int looptpool::getscale(){
    int sum=0,avg_scale=0;
    for(auto &ep:loadvec_){
        sum+=ep->getload();
    }
    if(sum==0) return 0;
    avg_scale=sum/t_num;
    return (avg_scale/sum)*100;
}



void looptpool::adjust_task(){
    while(dispath_){
        std::this_thread::sleep_for(std::chrono::seconds(timesec_));
        if(!dispath_) break;
        if(t_num>min_tnum&&t_num<max_tnum){
            int scale=getscale();
            if(scale<scale_min){
                delloop_dispatch();
                timesec_+=coolsec_;
            }else if(scale>scale_max){
                addloop();
                timesec_-=coolsec_;
            }
        }
        if(timesec_<ADJUST_TIMEOUT_SEC) timesec_=ADJUST_TIMEOUT_SEC;
    }
}



void looptpool::enable_adjust(){
    if(!manager_.joinable()){
        dispath_=true;
        init_adjust();
    }
}


eventloop* looptpool::getminload() {
    int min_load=loadvec_[0]->getload();
    int idx=0;
    unsigned int size=t_num;
    for(int i=1;i<size;++i){
        int cur_load=loadvec_[i]->getload();
        if(cur_load<min_load){
            min_load=cur_load;
            idx=i;
        }
    }
    return loadvec_[idx];
}


int looptpool::getmaxidx() {
    int max_load=loadvec_[0]->getload();
    int idx=0;
    unsigned int size=t_num;
    for(int i=1;i<t_num;++i){
        int cur_load=loadvec_[i]->getload();
        if(cur_load>max_load){
            max_load=cur_load;
            idx=i;
        }
    }
    return idx;
}



void looptpool::stop() {
    for(auto& ep : loadvec_) {
        ep->loopbreak();
        ep->getbaseloop()->join();
    }
    if(manager_.joinable()){
        stop_adjust();
        manager_.join();
    }
}
#ifndef _TIMEREVENT_H_
#define _TIMEREVENT_H_

#include "base_event.h"
#include <functional>

namespace moon{

    class event;
    class eventloop;

    class timerevent:public base_event{
    public:
        using Callback=std::function<void()>;
        /**
         * @brief 构造函数
         * @param loop 关联的事件循环
         * @param timeout_ms 定时器超时时间（毫秒）
         * @param periodic 是否为周期性定时器
         * @param cb 定时器回调函数
         */
         timerevent(eventloop* loop,int timeout_ms,bool periodic);
         ~timerevent();
         int getfd() const;
         eventloop* getloop() const override;
         void setcb(const Callback& cb);
         void start();
         void stop();
         void close() override{
            stop();
         }
         void disable_cb() override;
         Callback getcb();
    private:
        void  _init_();
        void handle_timeout();
    private:
        eventloop *loop_;
        event* ev_;
        int fd_;
        int timeout_ms_;
        bool periodic_;
        Callback cb_;
    };
}


#endif

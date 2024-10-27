// signalevent.h

#ifndef _SIGNALEVENT_H_
#define _SIGNALEVENT_H_

#include "base_event.h"
#include <functional>
#include <unistd.h>
#include <vector>
#include <signal.h>

namespace moon {

    class eventloop;
    class event;

    class signalevent : public base_event {
    public:
        using Callback = std::function<void(int)>;

        signalevent(eventloop* base);
        ~signalevent();
        eventloop* getloop() const override;
        void add_signal(int signo);
        void add_signal(const std::vector<int>& signals);
        void setcb(const Callback& cb);

        void enable_listen();
        void del_listen();
        void disable_cb() override;
        void close() override {
            del_listen();
        }
        Callback getcb();

    private:
        static void handle_signal(int signo);
        void handle_read();

    private:
        eventloop* loop_;
        int pipe_fd_[2]; // 管道的读写端
        event* ev_;
        Callback cb_;
        static signalevent* sigev_; // 单例实例
    };

}

#endif // !_SIGNALEVENT_H_

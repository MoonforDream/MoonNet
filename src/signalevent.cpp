// signalevent.cpp

#include "signalevent.h"
#include "event.h"
#include "eventloop.h"
#include <cstring>

using namespace moon;

signalevent* signalevent::sigev_ = nullptr;

signalevent::signalevent(eventloop* base) : loop_(base) {
    // 创建管道
    if (pipe(pipe_fd_) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // 设置管道为非阻塞模式
    setnonblock(pipe_fd_[0]);
    setnonblock(pipe_fd_[1]);

    ev_ = new event(loop_, pipe_fd_[0], EPOLLIN);
    ev_->setcb(std::bind(&signalevent::handle_read, this), nullptr, nullptr);
//    enable_listen();

    // 保存实例指针，供信号处理函数使用
    sigev_ = this;
}

signalevent::~signalevent() {
    if(ev_){
        delete ev_;
        ev_= nullptr;
    }
    ::close(pipe_fd_[0]);
    ::close(pipe_fd_[1]);
    sigev_ = nullptr;
}


eventloop* signalevent::getloop() const {
    return loop_;
}



void signalevent::add_signal(int signo) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signalevent::handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(signo, &sa, nullptr) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

void signalevent::add_signal(const std::vector<int>& signals) {
    for (auto sig : signals) {
        add_signal(sig);
    }
}

void signalevent::setcb(const Callback& cb) {
    cb_ = cb;
}

void signalevent::enable_listen() {
    if(ev_)
        ev_->enable_listen();
}

void signalevent::del_listen() {
    if(ev_)
        ev_->del_listen();
}

void signalevent::handle_signal(int signo) {
    // 在信号处理函数中，向管道写入信号编号
    uint8_t sig = static_cast<uint8_t>(signo);
    if (sigev_) {
        ssize_t n = write(sigev_->pipe_fd_[1], &sig, sizeof(sig));
        (void)n; // 忽略写入的返回值
    }
}

void signalevent::handle_read() {
    uint8_t signals[1024];
    ssize_t n = read(pipe_fd_[0], signals, sizeof(signals));
    if (n > 0) {
        for (ssize_t i = 0; i < n; ++i) {
            int sig = signals[i];
            if (cb_) cb_(sig);
        }
    }
}


void signalevent::disable_cb() {
    cb_= nullptr;
}


signalevent::Callback signalevent::getcb() {
    return cb_;
}
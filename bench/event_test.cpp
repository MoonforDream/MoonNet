// echo_server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <event2/event.h>
#include <pthread.h>
#include <iostream>

#define PORT 5005
#define NUM_WORKERS 4
#define BUFFER_SIZE 8192

typedef struct worker {
    struct event_base *base;
    struct event *notify_event;
    int notify_receive_fd;
    int notify_send_fd;
} worker_t;

typedef struct task {
    int fd;
} task_t;

worker_t workers[NUM_WORKERS];
int next_worker = 0;

// 读取客户端数据并回显
void echo_read_cb(evutil_socket_t fd, short events, void *arg) {
    char buffer[BUFFER_SIZE];
    ssize_t n = read(fd, buffer, sizeof(buffer));
    if (n <= 0) {
        if (n < 0)
            perror("read");
        close(fd);
        return;
    }
    // 回显数据
    //std::cout<<"recvied "<<fd<<" message :hello world"<<std::endl;
    write(fd, buffer, n);
    //std::cout<<"send success"<<std::endl;
}

// 从 Reactor 的事件循环
void *worker_thread(void *arg) {
    worker_t *worker = (worker_t *)arg;
    worker->base = event_base_new();

    // 设置接收主 Reactor 传递过来的新连接
    struct event *notify_event = event_new(worker->base, worker->notify_receive_fd, EV_READ | EV_PERSIST,
                                           [](evutil_socket_t fd, short what, void *arg) {
                                               worker_t *worker = (worker_t *)arg;
                                               int client_fd;
                                               ssize_t n = read(fd, &client_fd, sizeof(client_fd));
                                               if (n == sizeof(client_fd)) {
                                                   // 为新连接分配读事件
                                                   struct event *read_event = event_new(worker->base, client_fd, EV_READ | EV_PERSIST,
                                                                                        echo_read_cb, NULL);
                                                   event_add(read_event, NULL);
                                               }
                                           }, worker);
    event_add(notify_event, NULL);
    worker->notify_event = notify_event;

    event_base_dispatch(worker->base);
    event_base_free(worker->base);
    return NULL;
}

// 将新连接分配给一个从 Reactor
void dispatch(int client_fd) {
    worker_t *worker = &workers[next_worker];
    next_worker = (next_worker + 1) % NUM_WORKERS;
    // 发送客户端 fd 到从 Reactor
    write(worker->notify_send_fd, &client_fd, sizeof(client_fd));
}

// 监听新连接的回调
void accept_cb(evutil_socket_t listener, short event, void *arg) {
    struct event_base *base = (struct event_base *)arg;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(listener, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept");
        return;
    }
    // 设置为非阻塞
    evutil_make_socket_nonblocking(client_fd);
    // 分配给从 Reactor 处理
    dispatch(client_fd);
}

// 创建并绑定监听套接字
int create_listener(int port) {
    int listener;
    struct sockaddr_in sin;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        exit(1);
    }

    // 允许地址复用
    int reuse = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        close(listener);
        exit(1);
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(0); // 监听所有地址
    sin.sin_port = htons(port);

    if (bind(listener, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("bind");
        close(listener);
        exit(1);
    }

    if (listen(listener, 16) < 0) {
        perror("listen");
        close(listener);
        exit(1);
    }

    return listener;
}

int main(int argc, char **argv) {
    int listener = create_listener(PORT);
    evutil_make_socket_nonblocking(listener);

    // 初始化从 Reactor
    for (int i = 0; i < NUM_WORKERS; ++i) {
        int fds[2];
        if (pipe(fds)) {
            perror("pipe");
            return 1;
        }
        workers[i].notify_receive_fd = fds[0];
        workers[i].notify_send_fd = fds[1];
        // 创建工作线程
        pthread_t tid;
        pthread_create(&tid, NULL, worker_thread, &workers[i]);
        pthread_detach(tid);
    }

    // 创建主 Reactor
    struct event_base *base = event_base_new();
    struct event *listener_event = event_new(base, listener, EV_READ | EV_PERSIST, accept_cb, base);
    event_add(listener_event, NULL);

    printf("Echo server started on port %d\n", PORT);
    event_base_dispatch(base);

    // 清理
    event_free(listener_event);
    event_base_free(base);
    close(listener);
    return 0;
}

# MoonNet 使用手册

---

## 概述 (Overview)

MoonNet 是一个基于 C++ 的轻量级、高性能、事件驱动的网络库。它旨在简化网络应用程序的开发，支持 TCP 和 UDP 协议，并提供高效的事件循环机制，用于处理异步 I/O 操作。通过模块化设计，MoonNet 提供了灵活的接口，使开发者能够快速构建高性能的网络服务。

---

## 目录 (Table of Contents)

1. [模块概述 (Module Overview)](#模块概述-module-overview)
2. [类与接口 (Classes and Interfaces)](#类与接口-classes-and-interfaces)
   - [base_event](#base_event)
   - [event](#event)
   - [eventloop](#eventloop)
   - [loopthread](#loopthread)
   - [looptpool](#looptpool)
   - [threadpool](#threadpool)
   - [buffer](#buffer)
   - [bfevent](#bfevent)
   - [udpevent](#udpevent)
   - [timerevent](#timerevent)
   - [signalevent](#signalevent)
   - [acceptor](#acceptor)
   - [server](#server)
   - [wrap](#wrap)
   - [ringbuff](#ringbuff)
   - [lfthread](#lfthread)
   - [lfthreadpool](#lfthreadpool)
3. [使用示例 (Usage Examples)](#使用示例-usage-examples)
   - [TCP 服务器示例 (TCP Server Example)](#tcp-服务器示例-tcp-server-example)
   - [UDP 服务器示例 (UDP Server Example)](#udp-服务器示例-udp-server-example)
   - [定时器示例 (Timer Example)](#定时器示例-timer-example)
   - [信号处理示例 (Signal Handling Example)](#信号处理示例-signal-handling-example)
   - [ringbuff 使用示例 (RingBuff Usage Example)](#ringbuff-使用示例ringbuff-usage-example)
   - [lfthreadpool 使用示例 (LFThreadpool Usage Example)](#lfthreadpool-使用示例lfthreadpool-usage-example)
4. [错误处理 (Error Handling)](#错误处理-error-handling)
5. [常见问题 (FAQs)](#常见问题-faqs)
6. [结语 (Conclusion)](#结语-conclusion)

---

## 模块概述 (Module Overview)

MoonNet 的核心模块包括：

- **事件循环 (`eventloop`)**：管理事件的注册、删除和分发，核心的 Reactor 模型实现。
- **事件 (`event`)**：表示文件描述符的事件，封装了事件的回调和触发机制。
- **缓冲区 (`buffer`)**：用于处理非阻塞 I/O 的数据缓冲区，实现数据的缓存和处理。
- **缓冲事件 (`bfevent`)**：基于缓冲区的事件处理类，封装了读写缓冲区和回调函数。
- **UDP 事件 (`udpevent`)**：处理 UDP 协议的数据包收发。
- **定时器事件 (`timerevent`)**：提供定时器功能，支持一次性和周期性定时器。
- **信号事件 (`signalevent`)**：处理 UNIX 信号，将信号事件集成到事件循环中。
- **连接器 (`acceptor`)**：监听 TCP 端口并接受新连接。
- **服务器 (`server`)**：封装了 TCP 和 UDP 服务器功能，管理连接、事件和线程池。
- **线程池(`threadpool`)**：实现一个简单线程池，提供静态或动态模式，提供简单易用的接口函数
- **无锁环形缓冲区(`ringbuff`)**：实现了lock-free的环形缓冲区，提供简单易用的接口函数
- **无锁任务线程(`lfthread`)**：将`ringbuff`作为任务队列与线程封装成lfthread类，便于管理
- **无锁线程池(`lfthreadpool`)**：基于一个线程一个`ringbuff`作为任务队列的架构(封装成`lfthread`)，实现了lock-free的线程池，提供静态或动态模式，实现了动态退避的线程休眠调整策略，以及任务队列满后的拒绝策略

> **注意**：标记类似为`/** v1.0.0 **/`的注释部分包含已弃用或以前版本的事件处理函数。这些已被通用的函数所取代，为事件管理提供了一种统一的方法。

---

## 类与接口 (Classes and Interfaces)

### `base_event`

**描述 (Description):**

`base_event` 是所有事件类型的基类，定义了事件的基本接口和行为。

**接口 (Interface):**

```cpp
namespace moon {

class eventloop;
class base_event {
public:
    virtual ~base_event(){}
    virtual eventloop* getloop() const=0;
    virtual void close()=0;
    virtual void disable_cb()=0;
    /** v1.0.1 **/
    virtual void enable_listen()=0;
    virtual void del_listen()=0;
    virtual void update_ep()=0;
};

}
```

**函数说明 (Function Description):**

- `virtual eventloop* getloop() const = 0;`  
  获取关联的事件循环对象。

- `virtual void close() = 0;`  
  关闭事件，释放相关资源。

- `virtual void disable_cb() = 0;`  
  禁用事件的回调函数，防止事件被再次触发。
  
- `virtual void enable_listen() = 0;`

  开启事件监听

- `virtual void del_listen() = 0;`

  关闭事件监听

- `virtual void update_ep() = 0;`

  更新事件

---

### `event`

**描述 (Description):**

`event` 类表示一个文件描述符上的事件，封装了事件的类型、回调函数和触发机制。

**接口 (Interface):**

```cpp
namespace moon {

class eventloop;

// 事件类
class event : public base_event {
public:
    using Callback = std::function<void()>;

    event(eventloop* base, int fd, uint32_t events);
    ~event();

    int getfd() const;                // 获取文件描述符
    uint32_t getevents() const;       // 获取监听的事件类型
    eventloop* getloop() const override;

    void setcb(const Callback& rcb, const Callback& wcb, const Callback& ecb); // 设置回调函数
    void setrcb(const Callback& rcb); // 设置读事件回调
    void setwcb(const Callback& wcb); // 设置写事件回调
    void setecb(const Callback& ecb); // 设置错误事件回调

    void setrevents(const uint32_t revents); // 设置触发的事件类型
    void enable_events(uint32_t op);  // 启用指定的事件类型
    void disable_events(uint32_t op); // 禁用指定的事件类型
    void update_ep() override;                 // 更新监听事件
    void handle_cb();                 // 处理事件回调

    bool readable();                  // 检查是否可读
    bool writeable();                 // 检查是否可写
    void enable_read();               // 启用读事件
    void disable_read();              // 禁用读事件
    void enable_write();              // 启用写事件
    void disable_write();             // 禁用写事件
    void enable_ET();                 // 启用边缘触发模式
    void disable_ET();                // 禁用边缘触发模式

    void reset_events();              // 重置事件类型
    void del_listen() override;                // 删除监听
    void enable_listen() override;             // 启用监听
    void disable_cb() override;       // 禁用回调函数
    void close() override;            // 关闭事件

private:
    eventloop* loop_;
    int fd_;
    uint32_t events_;   // 监听的事件
    uint32_t revents_;  // 触发的事件
    Callback readcb_;   // 读事件回调
    Callback writecb_;  // 写事件回调
    Callback eventcb_;  // 错误事件回调
};

}
```

**函数说明 (Function Description):**

- `event(eventloop* base, int fd, uint32_t events);`  
  构造函数，初始化事件对象。

- `~event();`  
  析构函数，释放资源。

- `int getfd() const;`  
  获取事件关联的文件描述符。

- `uint32_t getevents() const;`  
  获取事件的监听类型。

- `eventloop* getloop() const override;`  
  获取关联的事件循环对象。

- `void setcb(const Callback& rcb, const Callback& wcb, const Callback& ecb);`  
  设置读、写和错误事件的回调函数。

- `void setrcb(const Callback& rcb);`  
  设置读事件回调。

- `void setwcb(const Callback& wcb);`  
  设置写事件回调。

- `void setecb(const Callback& ecb);`  
  设置错误事件回调。

- `void setrevents(const uint32_t revents);`  
  设置触发的事件类型。

- `void enable_events(uint32_t op);`  
  启用指定的事件类型。

- `void disable_events(uint32_t op);`  
  禁用指定的事件类型。

- `void update_ep() override;`  
  更新事件在 epoll 中的状态。

- `void handle_cb();`  
  处理事件，调用相应的回调函数。

- `bool readable();`  
  检查事件是否可读。

- `bool writeable();`  
  检查事件是否可写。

- `void enable_read();`  
  启用读事件监听。

- `void disable_read();`  
  禁用读事件监听。

- `void enable_write();`  
  启用写事件监听。

- `void disable_write();`  
  禁用写事件监听。

- `void enable_ET();`  
  启用边缘触发模式。

- `void disable_ET();`  
  禁用边缘触发模式。

- `void reset_events();`  
  重置事件类型。

- `void del_listen() override;`  
  删除事件监听。

- `void enable_listen() override;`  
  启用事件监听。

- `void disable_cb() override;`  
  禁用事件的回调函数。

- `void close() override;`  
  关闭事件。

---

### `eventloop`

**描述 (Description):**

`eventloop` 类是事件循环的核心，实现了 Reactor 模型，管理所有事件的注册、删除和分发。

**接口 (Interface):**

```cpp
namespace moon {

class base_event;
class event;
class loopthread;

// 事件循环类
class eventloop {
public:
    using Callback = std::function<void()>;

    eventloop(loopthread* base = nullptr, int timeout = -1);
    ~eventloop();

    loopthread* getbaseloop();
    int getefd() const;
    int getevfd() const;
    int getload() const;

    // 事件控制函数
    void add_event(event* event);
    void del_event(event* event);
    void mod_event(event* event);

    void loop();          // 开始事件循环
    void loopbreak();     // 终止事件循环
    void getallev(std::list<event*>& list);

    void create_eventfd();     // 创建通知文件描述符
    void read_eventfd();
    void write_eventfd();

    void add_pending_del(base_event* ev); // 添加待删除的事件

private:
    void updateload(int n); // 更新负载

private:
    int epfd_;                         // epoll 文件描述符
    int eventfd_;                      // 事件通知文件描述符
    int timeout_;                      // epoll 超时时间
    std::atomic<int> load_;            // 负载（活跃事件数）
    std::atomic<bool> shutdown_;       // 是否关闭事件循环
    std::list<event*> evlist_;         // 事件列表
    std::vector<epoll_event> events_;  // epoll 事件数组
    std::vector<base_event*> delque_;  // 待删除事件队列
    loopthread* baseloop_;             // 所属的线程
};

}
```

**函数说明 (Function Description):**

- `eventloop(loopthread* base = nullptr, int timeout = -1);`  
  构造函数，初始化事件循环对象。

- `~eventloop();`  
  析构函数，释放资源。

- `loopthread* getbaseloop();`  
  获取所属的线程对象。

- `int getefd() const;`  
  获取 epoll 文件描述符。

- `int getevfd() const;`  
  获取事件通知文件描述符。

- `int getload() const;`  
  获取当前事件循环的负载（活跃事件数）。

- `void add_event(event* event);`  
  添加事件到 epoll 监听。

- `void del_event(event* event);`  
  从 epoll 中删除事件。

- `void mod_event(event* event);`  
  修改事件的监听类型。

- `void loop();`  
  开始事件循环，处理事件。

- `void loopbreak();`  
  终止事件循环。

- `void getallev(std::list<event*>& list);`  
  获取所有事件列表。

- `void create_eventfd();`  
  创建事件通知文件描述符。

- `void read_eventfd();`  
  读取事件通知文件描述符，处理终止事件循环的信号。

- `void write_eventfd();`  
  写入事件通知文件描述符，通知事件循环终止。

- `void add_pending_del(base_event* ev);`  
  添加待删除的事件到队列。

---

### `loopthread`

**描述 (Description):**

`loopthread` 类封装了一个事件循环线程，用于运行 `eventloop`。

**接口 (Interface):**

```cpp
namespace moon {

class eventloop;

class loopthread {
public:
    loopthread(int timeout = -1);
    ~loopthread();

    eventloop* getloop(); // 获取事件循环对象

private:
    void _init_(); // 初始化事件循环

private:
    eventloop* loop_;
    std::thread t_;
    std::mutex mx_;
    std::condition_variable cv_;
    int timeout_;
};

}
```

**函数说明 (Function Description):**

- `loopthread(int timeout = -1);`  
  构造函数，初始化线程并创建事件循环。

- `~loopthread();`  
  析构函数，终止线程并释放资源。

- `eventloop* getloop();`  
  获取事件循环对象。

- `void _init_();`  
  内部函数，初始化事件循环并开始循环。

---

### `looptpool`

**描述 (Description):**

`looptpool` 类管理一组事件循环线程（`loopthread`），实现线程池功能，并提供静态/动态负载均衡。

**接口 (Interface):**

```cpp
namespace moon {

class eventloop;
class loopthread;

class looptpool {
public:
    looptpool(eventloop* base, bool dispath = false); // 默认不开启动态负载均衡
    ~looptpool();

    void create_pool(int timeout = -1);                  // 创建线程池
    void create_pool(int n, int timeout);                // 指定线程数创建线程池
    void create_pool_noadjust(int n, int timeout);       // 指定线程数创建线程池，不进行调度管理

    eventloop* ev_dispatch();    // 分发事件到线程池中的事件循环
    void delloop_dispatch();     // 删除从 reactor 并分发事件
    void addloop();              // 添加新的事件循环线程
    void adjust_task();          // 管理线程任务，动态调度从 reactor
    int getscale();              // 获取平均负载
    void enable_adjust();        // 启用动态负载均衡
    void stop();                 // 停止线程池

    // 禁用复制和赋值操作
    looptpool(const looptpool&) = delete;
    looptpool& operator=(const looptpool&) = delete;

private:
    void init_pool(int timeout = -1); // 初始化线程池
    eventloop* getminload();          // 获取负载最小的事件循环
    int getmaxidx();                  // 获取负载最大的事件循环的索引

private:
    eventloop* baseloop_;            // 主事件循环
    std::thread manager_;            // 管理线程
    std::vector<eventloop*> loadvec_;// 事件循环列表
    int next_;                       // 轮询索引
    int t_num_;                      // 线程数
    int timeout_;                    // 超时时间
    int max_tnum_;                   // 最大线程数
    int min_tnum_;                   // 最小线程数
    bool dispath_;                   // 是否启用动态调度
    int coolsec_;                    // 冷却时间
    int timesec_;                    // 调度时间间隔
    int scale_max_;                  // 最大负载比例
    int scale_min_;                  // 最小负载比例
};

}
```

**函数说明 (Function Description):**

- `looptpool(eventloop* base, bool dispath = false);`  
  构造函数，初始化线程池，是否启用动态负载均衡。

- `~looptpool();`  
  析构函数，释放资源。

- `void create_pool(int timeout = -1);`  
  创建线程池，使用默认线程数。

- `void create_pool(int n, int timeout);`  
  指定线程数创建线程池。

- `void create_pool_noadjust(int n, int timeout);`  
  指定线程数创建线程池，不进行动态调度。

- `eventloop* ev_dispatch();`  
  分发事件到线程池中的事件循环。

- `void delloop_dispatch();`  
  删除负载最大的事件循环并分发其事件。

- `void addloop();`  
  添加新的事件循环线程。

- `void adjust_task();`  
  管理线程任务，动态调整线程池大小。

- `int getscale();`  
  获取平均负载比例。

- `void enable_adjust();`  
  启用动态负载均衡。

- `void stop();`  
  停止线程池，终止所有事件循环。

---

### `threadpool`

**描述 (Description):**

`threadpool` 类实现了一个通用的线程池，用于执行任意的任务函数。

**接口 (Interface):**

```cpp
namespace moon {

class threadpool {
public:
    threadpool(int num);
    ~threadpool();

    // 添加任务到线程池
    template<typename _Fn, typename... _Args>
    void add_task(_Fn&& fn, _Args&&... args);

private:
    void init();          // 初始化线程池
    void t_shutdown();    // 销毁线程池
    void t_task();        // 任务线程入口函数
    void adjust_task();   // 管理线程入口函数

private:
    std::thread adjust_thr;                    // 管理线程
    std::vector<std::thread> threads;          // 工作线程
    std::queue<std::function<void()>> tasks;   // 任务队列
    std::mutex mx;                             // 互斥锁
    std::condition_variable task_cv;           // 条件变量
    int min_thr_num;                           // 最小线程数
    int max_thr_num;                           // 最大线程数
    std::atomic<int> run_num;                  // 正在执行任务的线程数
    std::atomic<int> live_num;                 // 存活线程数
    std::atomic<int> exit_num;                 // 要销毁的线程数
    bool shutdown;                             // 线程池是否关闭
};

}
```

**函数说明 (Function Description):**

- `threadpool(int num);`  
  构造函数，初始化线程池，指定最小线程数。

- `~threadpool();`  
  析构函数，销毁线程池。

- `void add_task(_Fn&& fn, _Args&&... args);`  
  添加任务到线程池，任务函数和参数。

- `void init();`  
  初始化线程池，创建工作线程和管理线程。

- `void t_shutdown();`  
  销毁线程池，终止所有线程。

- `void t_task();`  
  工作线程的入口函数，执行任务。

- `void adjust_task();`  
  管理线程的入口函数，动态调整线程数。

---

### `buffer`

**描述 (Description):**

`buffer` 类实现了一个可自动扩展的缓冲区，用于处理非阻塞 I/O 数据的缓存和操作。

**接口 (Interface):**

```cpp
namespace moon {

class buffer {
public:
    buffer();
    ~buffer();

    void append(const char* data, size_t len); // 向缓冲区追加数据
    size_t remove(char* data, size_t len);     // 从缓冲区读取数据
    std::string remove(size_t len);            // 从缓冲区读取数据并返回字符串
    void retrieve(size_t len);                 // 仅移动读指针，标记数据已读
    size_t readbytes() const;                  // 获取可读数据大小
    size_t writebytes() const;                 // 获取可写空间大小
    const char* peek() const;                  // 获取当前读指针位置的数据
    void reset();                              // 重置缓冲区
    ssize_t readiov(int fd, int& errnum);      // 从文件描述符读取数据到缓冲区

private:
    void able_wirte(size_t len);               // 确保有足够的可写空间

private:
    std::vector<char> buffer_;
    uint64_t reader_;    // 读指针位置
    uint64_t writer_;    // 写指针位置
};

}
```

**函数说明 (Function Description):**

- `buffer();`  
  构造函数，初始化缓冲区。

- `~buffer();`  
  析构函数，释放资源。

- `void append(const char* data, size_t len);`  
  向缓冲区追加数据。

- `size_t remove(char* data, size_t len);`  
  从缓冲区读取数据到指定内存。

- `std::string remove(size_t len);`  
  从缓冲区读取数据并返回字符串。

- `void retrieve(size_t len);`  
  仅移动读指针，标记数据已读。

- `size_t readbytes() const;`  
  获取可读数据的字节数。

- `size_t writebytes() const;`  
  获取可写空间的字节数。

- `const char* peek() const;`  
  获取当前读指针位置的数据指针。

- `void reset();`  
  重置缓冲区，清空数据。

- `ssize_t readiov(int fd, int& errnum);`  
  从文件描述符读取数据到缓冲区。

---

### `bfevent`

**描述 (Description):**

`bfevent` 类是基于缓冲区的事件处理类，封装了读写缓冲区和回调函数，处理 TCP 连接的数据收发。

**接口 (Interface):**

```cpp
namespace moon {

class eventloop;
class event;

class bfevent : public base_event {
public:
    using RCallback = std::function<void(bfevent*)>;
    using Callback = std::function<void()>;

    bfevent(eventloop* base, int fd, uint32_t events);
    ~bfevent();

    int getfd() const;
    eventloop* getloop() const override;
    buffer* getinbuff();
    buffer* getoutbuff();
    bool writeable() const;

    void setcb(const RCallback& rcb, const Callback& wcb, const Callback& ecb); // 设置回调函数
    void setrcb(const RCallback& rcb);
    void setwcb(const Callback& wcb);
    void setecb(const Callback& ecb);
    RCallback getrcb();
    Callback getwcb();
    Callback getecb();

    void update_ep() override;                // 更新监听事件
    void del_listen() override;               // 取消监听
    void enable_listen() override;            // 启用监听

    void sendout(const char* data, size_t len);   // 发送数据
    void sendout(const std::string& data);        // 发送数据
    size_t receive(char* data, size_t len);       // 接收数据到指定内存
    std::string receive(size_t len);              // 接收指定长度的数据
    std::string receive();                        // 接收所有可读数据

    void enable_events(uint32_t op);  // 启用指定事件
    void disable_events(uint32_t op); // 禁用指定事件
    void enable_read();               // 启用读事件
    void disable_read();              // 禁用读事件
    void enable_write();              // 启用写事件
    void disable_write();             // 禁用写事件
    void enable_ET();                 // 启用边缘触发
    void disable_ET();                // 禁用边缘触发
    void disable_cb() override;       // 禁用回调函数

    void close() override;            // 关闭事件

private:
    void close_event();               // 内部函数，关闭事件
    void handle_read();               // 处理读事件
    void handle_write();              // 处理写事件
    void handle_event();              // 处理错误事件

private:
    eventloop* loop_;
    int fd_;
    event* ev_;
    buffer inbuff_;
    buffer outbuff_;
    RCallback readcb_;
    Callback writecb_;
    Callback eventcb_;
    bool closed_;
};

}
```

**函数说明 (Function Description):**

- `bfevent(eventloop* base, int fd, uint32_t events);`  
  构造函数，初始化缓冲事件对象。

- `~bfevent();`  
  析构函数，关闭事件并释放资源。

- `int getfd() const;`  
  获取文件描述符。

- `eventloop* getloop() const override;`  
  获取关联的事件循环。

- `buffer* getinbuff();`  
  获取输入缓冲区。

- `buffer* getoutbuff();`  
  获取输出缓冲区。

- `bool writeable() const;`  
  检查是否可写。

- `void setcb(const RCallback& rcb, const Callback& wcb, const Callback& ecb);`  
  设置读、写、错误事件的回调函数。

- `void update_ep() override;`  
  更新监听事件。

- `void del_listen() override;`  
  取消监听。

- `void enable_listen() override;`  
  启用监听。

- `void sendout(const char* data, size_t len);`  
  发送数据。

- `void sendout(const std::string& data);`  
  发送数据。

- `size_t receive(char* data, size_t len);`  
  接收数据到指定内存。

- `std::string receive(size_t len);`  
  接收指定长度的数据。

- `std::string receive();`  
  接收所有可读数据。

- `void enable_events(uint32_t op);`  
  启用指定事件。

- `void disable_events(uint32_t op);`  
  禁用指定事件。

- `void enable_read();`  
  启用读事件。

- `void disable_read();`  
  禁用读事件。

- `void enable_write();`  
  启用写事件。

- `void disable_write();`  
  禁用写事件。

- `void enable_ET();`  
  启用边缘触发。

- `void disable_ET();`  
  禁用边缘触发。

- `void disable_cb() override;`  
  禁用回调函数。

- `void close() override;`  
  关闭事件。

---

### `udpevent`

**描述 (Description):**

`udpevent` 类处理 UDP 协议的数据收发，支持非阻塞的 UDP 通信。

**接口 (Interface):**

```cpp
namespace moon {

class eventloop;
class event;

// UDP 事件处理类
class udpevent : public base_event {
public:
    using Callback = std::function<void()>;
    using RCallback = std::function<void(const sockaddr_in&, udpevent*)>;

    udpevent(eventloop* base, int port);
    ~udpevent();

    buffer* getinbuff();
    eventloop* getloop() const override;
    void setcb(const RCallback& rcb, const Callback& ecb);
    void setrcb(const RCallback& rcb);
    void setecb(const Callback& ecb);
    void init_sock(int port);
    /** v1.0.0 **/
    //void start();       // 开始监听
    //void stop();        // 停止监听
    
    /** v1.0.1 **/
    void enable_listen() override;	// 开始监听
    void del_listen() override;		// 停止监听
    
	void update_ep() override;   // 更新监听事件
    size_t receive(char* data, size_t len);  // 接收数据到指定内存
    std::string receive(size_t len);         // 接收指定长度的数据
    std::string receive();                   // 接收所有可读数据

    void send_to(const std::string& data, const sockaddr_in& addr); // 发送数据到指定地址

    void enable_read();
    void disable_read();
    void enable_ET();
    void disable_ET();
    void disable_cb() override;
    RCallback getrcb();
    Callback getecb();
    void close() override; // 关闭事件

private:
    void handle_receive(); // 处理接收事件

private:
    eventloop* loop_;
    int fd_;
    event* ev_;
    buffer inbuff_;
    RCallback receive_cb_;
    Callback event_cb_;
    bool started_;
};

}
```

**函数说明 (Function Description):**

- `udpevent(eventloop* base, int port);`  
  构造函数，初始化 UDP 事件对象。

- `~udpevent();`  
  析构函数，关闭事件并释放资源。

- `buffer* getinbuff();`  
  获取输入缓冲区。

- `eventloop* getloop() const override;`  
  获取关联的事件循环。

- `void setcb(const RCallback& rcb, const Callback& ecb);`  
  设置接收和错误事件的回调函数。

- `void init_sock(int port);`  
  初始化 UDP 套接字。

- `void enable_listen() override;`  
  开始监听 UDP 数据包。

- `void del_listen() override;`  
  停止监听。

- `void update_ep() override;`  
  更新监听事件。

- `size_t receive(char* data, size_t len);`  
  从缓冲区接收数据。

- `std::string receive(size_t len);`  
  从缓冲区接收指定长度的数据。

- `std::string receive();`  
  从缓冲区接收所有可读数据。

- `void send_to(const std::string& data, const sockaddr_in& addr);`  
  发送数据到指定的地址。

- `void enable_read();`  
  启用读事件。

- `void disable_read();`  
  禁用读事件。

- `void enable_ET();`  
  启用边缘触发。

- `void disable_ET();`  
  禁用边缘触发。

- `void disable_cb() override;`  
  禁用回调函数。

- `void close() override;`  
  关闭事件。

---

### `timerevent`

**描述 (Description):**

`timerevent` 类实现了定时器功能，支持一次性和周期性定时器，用于定期执行任务。

**接口 (Interface):**

```cpp
namespace moon {

class event;
class eventloop;

class timerevent : public base_event {
public:
    using Callback = std::function<void()>;

    timerevent(eventloop* loop, int timeout_ms, bool periodic);
    ~timerevent();

    int getfd() const;
    eventloop* getloop() const override;
    void setcb(const Callback& cb);
    /** v1.0.0 **/
    //void start();
    //void stop();
    /** v1.0.1 **/
    void enable_listen() override;
    void del_listen() override;
    void update_ep() override;
    
    void close() override;
    void disable_cb() override;
    Callback getcb();

private:
    void _init_();
    void handle_timeout();

private:
    eventloop* loop_;
    event* ev_;
    int fd_;
    int timeout_ms_;
    bool periodic_;
    Callback cb_;
};

}
```

**函数说明 (Function Description):**

- `timerevent(eventloop* loop, int timeout_ms, bool periodic);`  
  构造函数，初始化定时器事件。

- `~timerevent();`  
  析构函数，关闭定时器并释放资源。

- `int getfd() const;`  
  获取定时器文件描述符。

- `eventloop* getloop() const override;`  
  获取关联的事件循环。

- `void setcb(const Callback& cb);`  
  设置定时器回调函数。

- `void enable_listen() override;`  
  启动定时器。

- `void del_listen() override;`  
  停止定时器。

- `void update_ep() override;`
  
  更新事件
  
- `void close() override;`  
  关闭定时器事件。

- `void disable_cb() override;`  
  禁用回调函数。

---

### `signalevent`

**描述 (Description):**

`signalevent` 类处理 UNIX 信号，将信号事件集成到事件循环中，通过管道机制实现。

**接口 (Interface):**

```cpp
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

    void enable_listen() override;
    void del_listen() override;
    void update_ep() override;
    void disable_cb() override;
    void close() override;
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
```

**函数说明 (Function Description):**

- `signalevent(eventloop* base);`  
  构造函数，初始化信号事件对象。

- `~signalevent();`  
  析构函数，关闭事件并释放资源。

- `void add_signal(int signo);`  
  添加单个信号监听。

- `void add_signal(const std::vector<int>& signals);`  
  添加多个信号监听。

- `void setcb(const Callback& cb);`  
  设置信号处理回调函数。

- `void enable_listen() override;`  
  启用信号监听。

- `void del_listen() override;`  
  停止信号监听。

- `void update_ep() override;`
  
  更新事件
  
- `void disable_cb() override;`  
  禁用回调函数。

- `void close() override;`  
  关闭信号事件。

---

### `acceptor`

**描述 (Description):**

`acceptor` 类负责监听指定端口，接受新的 TCP 连接，并将新连接交给回调函数处理。

**接口 (Interface):**

```cpp
namespace moon {

class eventloop;
class event;

// 连接器类
class acceptor {
public:
    using Callback = std::function<void(int)>;

    acceptor(int port, eventloop* base);
    ~acceptor();

    void listen();               // 开始监听
    void stop();                 // 停止监听
    void init_sock(int port);    // 初始化监听套接字
    void setcb(const Callback& accept_cb); // 设置回调函数
    void handle_accept();        // 处理新连接

private:
    int lfd_;           // 监听套接字
    eventloop* loop_;   // 事件循环
    event* ev_;         // 事件对象
    Callback cb_;       // 回调函数
    bool shutdown_;     // 是否已关闭
};

}
```

**函数说明 (Function Description):**

- `acceptor(int port, eventloop* base);`  
  构造函数，初始化连接器对象。

- `~acceptor();`  
  析构函数，关闭监听并释放资源。

- `void listen();`  
  开始监听。

- `void stop();`  
  停止监听。

- `void init_sock(int port);`  
  初始化监听套接字。

- `void setcb(const Callback& accept_cb);`  
  设置新连接到来的回调函数。

- `void handle_accept();`  
  处理新连接事件。

---

### `server`

**描述 (Description):**

`server` 类封装了 TCP 和 UDP 服务器功能，管理连接、事件和线程池。

**接口 (Interface):**

```cpp
namespace moon {

class udpevent;
class signalevent;
class timerevent;

class server {
public:
    using SCallback = std::function<void(int)>;
    using UCallback = std::function<void(const sockaddr_in&, udpevent*)>;
    using RCallback = std::function<void(bfevent*)>;
    using Callback = std::function<void()>;

    server(int port = -1);
    ~server();

    void start();                         // 启动服务器
    void stop();                          // 停止服务器
    void init_pool(int timeout = -1);     // 初始化线程池
    void init_pool(int tnum, int timeout);// 指定线程数初始化线程池
    void init_pool_noadjust(int tnum, int timeout); // 指定线程数初始化线程池，不进行动态调度
    void enable_tcp(int port);            // 启用 TCP 服务
    void enable_tcp_accept();             // 启用 TCP 连接监听
    void disable_tcp_accept();            // 禁用 TCP 连接监听
    eventloop* getloop();                 // 获取主事件循环
    eventloop* dispatch();                // 分发事件

    // 设置 TCP 连接的回调函数
    void set_tcpcb(const RCallback& rcb, const Callback& wcb, const Callback& ecb);

    // 事件操作
    void addev(base_event *ev);
    void delev(base_event *ev);
    void modev(base_event *ev);r
    udpevent* add_udpev(int port, const UCallback& rcb, const Callback& ecb);	//udpevent推荐使用这个函数添加，出错会自动清理
    signalevent* add_sev(int signo, const SCallback& cb);
    signalevent* add_sev(const std::vector<int>& signals, const SCallback& cb);
    timerevent* add_timeev(int timeout_ms, bool periodic, const Callback& cb);

    /** v1.0.0 **/
    /* void add_ev(event *ev);
    void del_ev(event *ev);
    void mod_ev(event *ev);
    void add_bev(bfevent *bev);
    void del_bev(bfevent *bev);
    void mod_bev(bfevent *bev);
    void add_udpev(udpevent *uev);
    void del_udpev(udpevent *uev);
    void mod_udpev(udpevent *uev);
    void add_sev(signalevent* sev);
    void del_sev(signalevent* sev);
    void add_timeev(timerevent *tev);
    void del_timeev(timerevent *tev); */

private:
    void acceptcb_(int fd); // 处理新连接

    void tcp_eventcb_(bfevent* bev); // 处理 TCP 错误事件

    void handle_close(base_event* ev); // 处理事件关闭

private:
    std::mutex events_mutex_;
    eventloop base_;             // 主事件循环
    looptpool pool_;             // 线程池
    acceptor acceptor_;          // 连接器
    int port_;                   // 端口号
    bool tcp_enable_;            // 是否启用 TCP
    std::list<base_event*> events_; // 事件列表

    // TCP 连接回调函数
    RCallback readcb_;
    Callback writecb_;
    Callback eventcb_;
};

}
```

**函数说明 (Function Description):**

- `server(int port = -1);`  
  构造函数，初始化服务器对象。

- `~server();`  
  析构函数，关闭服务器并释放资源。

- `void start();`  
  启动服务器，开始事件循环。

- `void stop();`  
  停止服务器，终止事件循环。

- `void init_pool(int timeout = -1);`  
  初始化线程池。

- `void enable_tcp(int port);`  
  启用 TCP 服务。

- `void enable_tcp_accept();`  
  启用 TCP 连接监听。

- `void disable_tcp_accept();`  
  禁用 TCP 连接监听。

- `eventloop* getloop();`  
  获取主事件循环。

- `eventloop* dispatch();`  
  分发事件到线程池。

- `void set_tcpcb(const RCallback& rcb, const Callback& wcb, const Callback& ecb);`  
  设置 TCP 连接的回调函数。

- `void addev(base_event *ev);`

  添加通用事件(可传任意事件类型)

- `void delev(base_event *ev);`

  删除通用事件(可传任意事件类型)

- `void modev(base_event *ev);`

  修改通用事件(可传任意事件类型)

- `udpevent* add_udpev(int port, const UCallback& rcb, const Callback& ecb);`  
  添加并初始化 UDP 事件。

- `signalevent* add_sev(int signo, const SCallback& cb);`  
  添加并初始化信号事件。

- `timerevent* add_timeev(int timeout_ms, bool periodic, const Callback& cb);`  
  添加并初始化定时器事件。

---

### `wrap`

**描述 (Description):**

`wrap` 模块封装了跨平台的套接字操作函数，为 Windows 和 Unix 系统提供统一的接口。

**接口 (Interface):**

```cpp
#ifndef _WRAP_H_
#define _WRAP_H_

#ifdef _WIN32

// Windows 平台的套接字封装函数
void setreuse(SOCKET fd);
void setnonblock(SOCKET fd);
void perr_exit(const char* s);
// 其他函数定义...

#else

// Unix/Linux 平台的套接字封装函数
void settcpnodelay(int fd);
void setreuse(int fd);
void setnonblock(int fd);
void perr_exit(const char* s);
int Accept(int fd, struct sockaddr* sa, socklen_t* salenptr);
int Bind(int fd, const struct sockaddr* sa, socklen_t salen);
int Connect(int fd, const struct sockaddr* sa, socklen_t salen);
int Listen(int fd, int backlog);
int Socket(int family, int type, int protocol);
ssize_t Read(int fd, void* ptr, size_t nbytes);
ssize_t Write(int fd, const void* ptr, size_t nbytes);
int Close(int fd);
ssize_t Readn(int fd, void* vptr, size_t n);
ssize_t Writen(int fd, const void* vptr, size_t n);
ssize_t Readline(int fd, void* vptr, size_t maxlen);
// 其他函数定义...

#endif

#endif
```

**函数说明 (Function Description):**

- `void settcpnodelay(int fd);`  
  设置套接字为无延迟模式（TCP_NODELAY）。

- `void setreuse(int fd);`  
  设置套接字地址和端口可重用。

- `void setnonblock(int fd);`  
  设置套接字为非阻塞模式。

- `void perr_exit(const char* s);`  
  打印错误信息并退出程序。

- `int Accept(int fd, struct sockaddr* sa, socklen_t* salenptr);`  
  接受新的连接。

- `int Bind(int fd, const struct sockaddr* sa, socklen_t salen);`  
  绑定套接字到指定地址和端口。

- `int Connect(int fd, const struct sockaddr* sa, socklen_t salen);`  
  连接到指定的地址和端口。

- `int Listen(int fd, int backlog);`  
  开始监听套接字。

- `int Socket(int family, int type, int protocol);`  
  创建新的套接字。

- `ssize_t Read(int fd, void* ptr, size_t nbytes);`  
  从套接字读取数据。

- `ssize_t Write(int fd, const void* ptr, size_t nbytes);`  
  向套接字写入数据。

- `int Close(int fd);`  
  关闭套接字。

- `ssize_t Readn(int fd, void* vptr, size_t n);`  
  从套接字读取指定字节数的数据。

- `ssize_t Writen(int fd, const void* vptr, size_t n);`  
  向套接字写入指定字节数的数据。

- `ssize_t Readline(int fd, void* vptr, size_t maxlen);`  
  从套接字读取一行数据。

---

### `ringbuff`

**描述 (Description):**

`ringbuff` 是一个无锁的环形缓冲区，设计用于高效、高性能的场景，适用于多线程同时生产和消费数据的情况。这种结构特别适合需要最小延迟和开销的实时数据处理应用。

**接口 (Interface):**

```cpp
namespace moon {

    // 无锁环形缓冲区 RingBuffer
    // lock-free ringbuffer
    template <class T>
    class ringbuff {
    public:
        ringbuff(size_t size = 1024)
            : head_(0),
              tail_(0),
              capacity_(adj_size(size)),
              buffer_(new T[capacity_]) {}
        //        ~ringbuff()=default;
        ~ringbuff() { delete[] buffer_; }
        /** push function **/
        bool push(const T& item);
        // push with move
        bool push_move(T&& item);
        /** pop function **/
        bool pop(T& item);
        // pop with move
        bool pop_move(T& item);

        /** performance function **/
        size_t capacity() const;
        size_t size() const;
        bool empty();
        bool full();
        void swap(ringbuff& other) noexcept;

        /** swap_to function **/
        void swap_to_list(std::list<T>& list_);
        void swap_to_vector(std::vector<T>& vec_);
        std::list<T> swap_to_list();
        std::vector<T> swap_to_vector();

    private:
        bool is_powtwo(size_t n) const;
        size_t next_powtwo(size_t n) const;
        size_t adj_size(size_t size) const;

    private:
        // avoid pseudo shareing
        std::atomic<size_t> head_ alignas(64);
        std::atomic<size_t> tail_ alignas(64);
        //        std::unique_ptr<T[]> buffer_;
        size_t capacity_;
        T* buffer_;
    };

}  // namespace moon
```

**函数说明 (Function Description):**

- `ringbuff(size_t size = 1024)`: 构造函数，初始化环形缓冲区并设定容量，默认容量为1024，容量会调整为最近的二次幂以优化性能。
- `~ringbuff()`: 析构函数，清理分配的缓冲区。
- `bool push(const T& item)`: 尝试向缓冲区头部添加一个元素。如果成功返回 `true`；如果缓冲区满了，则返回 `false`。此方法会复制元素。
- `bool push_move(T&& item)`: 类似于 `push`，但使用移动语义来避免复制元素。如果成功返回 `true`；如果缓冲区满了，则返回 `false`。
- `bool pop(T& item)`: 尝试从缓冲区尾部移除一个元素，并将其复制到提供的变量中。如果成功返回 `true`；如果缓冲区空，则返回 `false`。
- `bool pop_move(T& item)`: 类似于 `pop`，但使用移动语义来移动元素。如果成功返回 `true`；如果缓冲区空，则返回 `false`。
- `size_t capacity() const`: 返回缓冲区的当前容量。
- `size_t size() const`: 返回缓冲区中当前的元素数量，**该操作需要额外的原子操作，性能损失，请谨慎使用。**
- `bool empty() const`: 检查缓冲区是否为空。
- `bool full() const`: 检查缓冲区是否已满。
- `void swap(ringbuff& other) noexcept`: 与另一个 `ringbuff` 实例交换内容，包括它们的内部状态，无需复制元素。
- `void swap_to_list(std::list<T>& list_)`: 将缓冲区中的所有元素移动到提供的 `std::list` 中，之后清空缓冲区。
- `void swap_to_vector(std::vector<T>& vec_)`: 将缓冲区中的所有元素移动到提供的 `std::vector` 中，之后清空缓冲区。
- `std::list<T> swap_to_list()`: 创建一个新的 `std::list`，将缓冲区中的所有元素移动进去，返回这个列表。
- `std::vector<T> swap_to_vector()`: 创建一个新的 `std::vector`，将缓冲区中的所有元素移动进去，返回这个向量。

---

### `lfthread`

**描述 (Description):**

`lfthread` 是一个轻量级、无锁的线程管理类，专为高并发和低延迟应用设计。它使用环形缓冲区 (`ringbuff`) 来队列任务，这些任务被封装在 `std::function<void()>` 中。该类在其自有线程中处理任务执行，并提供任务入队、线程关闭和缓冲区交换的机制。

 **接口 (Interface):**

```cpp
namespace moon {

    class lfthread {
    public:
        using task = std::function<void()>;
        lfthread(size_t size)
            : buffer_(size),
              shutdown_(false),
              t_(std::thread(&lfthread::t_task, this)) {}
        ~lfthread() { t_shutdown(); }
        bool enqueue_task(task _task);
        bool enqueue_task_move(task&& _task);
        void t_shutdown();
        int getload() const;
        void swap_to_ringbuff(ringbuff<task>& rb_);
        void swap_to_list(std::list<task>& list_);
        void swap_to_vector(std::vector<task>& vec_);
        std::list<task> swap_to_list();
        std::vector<task> swap_to_vector();

        /** move copy **/
        lfthread& operator=(lfthread&& other) noexcept;
        lfthread(lfthread&& other) noexcept;

    private:
        void t_task();

    private:
        ringbuff<task> buffer_;  // 任务队列(无锁环形缓冲区)
        bool shutdown_;  // 无任何其他线程操作，所以不需要原子变量
        std::thread t_;
    };

}  // namespace moon
```

**函数说明 (Function Description):**

- `lfthread(size_t size)`: 构造函数，为任务缓冲区指定大小。
- `~lfthread()`: 析构函数，确保线程正确关闭。
- `bool enqueue_task(task _task)`: 尝试将新任务入队到缓冲区。如果成功返回 `true`；如果缓冲区已满返回 `false`。
- `bool enqueue_task_move(task&& _task)`: 类似于 `enqueue_task`，但使用移动语义来优化性能。
- `void t_shutdown()`: 关闭线程，确保所有任务完成且线程可以加入，然后退出。
- `int getload() const`: 返回缓冲区中当前的任务数量。
- `void swap_to_ringbuff(ringbuff<task>& rb_)`: 与另一个环形缓冲区交换内部任务缓冲。
- `void swap_to_list(std::list<task>& list_)`: 将缓冲区中的所有任务转移至指定的 std::list。
- `void swap_to_vector(std::vector<task>& vec_)`: 将缓冲区中的所有任务转移至指定的 std::vector。
- `std::list<task> swap_to_list()`: 返回一个 std::list，包含来自缓冲区的所有任务。
- `std::vector<task> swap_to_vector()`: 返回一个 std::vector，包含来自缓冲区的所有任务。
- `lfthread& operator=(lfthread&& other) noexcept`: 移动赋值操作符。
- `lfthread(lfthread&& other) noexcept`: 移动构造函数。

---

### `lfthreadpool`

**描述 (Description):**

`lfthreadpool` 是一个多功能的线程池管理类，设计用来在动态或静态分配模式下管理任务执行的线程。它利用 `lfthread` 实例来管理单个线程和任务，允许在多线程环境中有效地分配和执行任务。线程池可以在静态模式（线程数量固定）或动态模式（根据工作负载调整线程数量）中操作。

 **接口 (Interface):**

```cpp
#define ADJUST_TIMEOUT_SEC 5

namespace moon {

    class lfthread;

    enum class PoolMode {
        Static,  // 静态模式
        Dynamic  // 动态模式
    };

    class lfthreadpool {
    public:
        lfthreadpool(int tnum = -1, size_t buffsize = 1024,
                     PoolMode mode = PoolMode::Static);
        ~lfthreadpool();
        void init();
        void t_shutdown();

        /** add_task function **/
        template <typename _Fn, typename... _Args>
        bool add_task(_Fn&& fn, _Args&&... args);

        template <typename _Fn, typename... _Args>
        bool add_task_move(_Fn&& fn, _Args&&... args);

        lfthreadpool(const lfthreadpool&) = delete;
        lfthreadpool& operator=(const lfthreadpool&) = delete;

    private:
        const size_t getnext();
        void adjust_task();
        void del_thread_dispath();
        void add_thread();
        const size_t getminidx();
        const size_t getmaxidx();
        const int getload();
        void setnum();
        void setnum(size_t n);
        int getcores() const;

    private:
        std::atomic<size_t> tnum_;
        // size_t tnum_;
        std::thread mentor_;
        std::vector<lfthread*> workers_;
        // bool shutdown_;
        std::atomic<bool> shutdown_;
        PoolMode mode_;
        size_t buffsize_;
        size_t next_ = 0;
        int timesec_ = 5;
        int coolsec_ = 30;
        int load_max = 80;
        int load_min = 20;
        int max_tnum = 0;
        int min_tnum = 0;
    };

}  // namespace moon
```

**函数说明 (Function Description):**

- **`lfthreadpool(int tnum, size_t buffsize, PoolMode mode)`**: 构造具有特定线程数量、每线程缓冲区大小及操作模式的线程池。
- **`~lfthreadpool()`**: 销毁线程池，确保所有线程都已正确关闭。
- **`void init()`**: 初始化线程池，创建指定数量的线程。
- **`void t_shutdown()`**: 安全地关闭池中的所有线程。
- **`bool add_task(_Fn&& fn, _Args&&... args)`**: 向池中添加新任务；任务根据当前负载和池模式分配给线程。
- **`bool add_task_move(_Fn&& fn, _Args&&... args)`**: 类似于 `add_task`，但使用移动语义来优化可移动任务的处理。
- **`void adjust_task()`**: 如果池处于动态模式，根据当前负载和预定义阈值动态调整线程数量。
- **`void del_thread_dispath()`**: 在不再需要时从池中移除线程，并将其任务重新分配给剩余线程。
- **`void add_thread()`**: 为处理增加的负载向池中添加新线程。
- **`int getload()`**: 计算所有线程的总负载作为其容量的百分比。

---

## 使用示例 (Usage Examples)

以下示例展示了如何使用 MoonNet 网络库构建 TCP 和 UDP 服务器，以及如何使用定时器和信号处理功能。

### TCP 服务器示例 (TCP Server Example)

**描述 (Description):**

创建一个简单的 TCP 服务器，监听指定端口，接收客户端连接，并回显接收到的数据。

**代码示例 (Code Example):**

```cpp
#include "server.h"
#include "bfevent.h"
#include <iostream>

// 读事件回调函数
void on_read(moon::bfevent* bev) {
    std::string data = bev->receive();
    if (!data.empty()) {
        std::cout << "Received: " << data << std::endl;
        // 回显数据
        bev->sendout(data);
    }
}

// 写事件回调函数
void on_write(){
    ...
}

// 其他事件回调函数（如错误）
void on_event() {
    std::cout << "An error occurred on the connection." << std::endl;
}

int main() {
    // 创建服务器，监听端口 8080
    moon::server tcp_server(8080);

    // 设置缓冲事件的回调函数
    tcp_server.set_tcpcb(on_read, on_write, on_event);

    // 初始化线程池（可选）
    tcp_server.init_pool();

    // 启动服务器
    tcp_server.start();

    return 0;
}
```

**说明 (Explanation):**

1. **创建服务器实例 (Create Server Instance):**

   ```cpp
   moon::server tcp_server(8080);
   ```

   创建一个 `server` 对象，监听 TCP 端口 `8080`。

2. **设置回调函数 (Set Callback Functions):**

   ```cpp
   tcp_server.set_tcpcb(on_read, nullptr, on_event);
   ```

   设置缓冲事件的读和错误事件的回调函数。

3. **初始化线程池 (Initialize Thread Pool):**

   ```cpp
   tcp_server.init_pool();
   ```

   初始化线程池，以提高服务器的并发处理能力。

4. **启动服务器 (Start Server):**

   ```cpp
   tcp_server.start();
   ```

   启动服务器，开始事件循环，监听和处理连接。

---

### UDP 服务器示例 (UDP Server Example)

**描述 (Description):**

创建一个简单的 UDP 服务器，监听指定端口，接收数据包并回复客户端。

**代码示例 (Code Example):**

```cpp
#include "server.h"
#include "udpevent.h"
#include <iostream>

// UDP 接收回调函数
void on_udp_receive(const sockaddr_in& addr, moon::udpevent* uev) {
    std::string data = uev->receive();
    if (!data.empty()) {
        std::cout << "UDP Received from " << inet_ntoa(addr.sin_addr)
                  << ":" << ntohs(addr.sin_port) << " - " << data << std::endl;

        // 发送回复
        std::string reply = "Echo: " + data;
        uev->send_to(reply, addr);
    }
}

int main() {
    // 创建服务器，不指定 TCP 端口
    moon::server udp_server(-1);

    // 添加 UDP 事件，监听端口 5005
    udp_server.add_udpev(5005, on_udp_receive, nullptr);

    // 启动服务器
    udp_server.start();

    return 0;
}
```

**说明 (Explanation):**

1. **创建服务器实例 (Create Server Instance):**

   ```cpp
   moon::server udp_server(-1);
   ```

   创建一个 `server` 对象，不启用 TCP 服务。

2. **添加 UDP 事件 (Add UDP Event):**

   ```cpp
   udp_server.add_udpev(5005, on_udp_receive, nullptr);
   ```

   添加一个 UDP 事件，监听端口 `5005`，并设置接收回调函数。

3. **启动服务器 (Start Server):**

   ```cpp
   udp_server.start();
   ```

   启动服务器，开始事件循环，监听和处理 UDP 数据包。

---

### 定时器示例 (Timer Example)

**描述 (Description):**

使用定时器定期执行任务，如每隔一秒打印一条消息。

**代码示例 (Code Example):**

```cpp
#include "server.h"
#include "timerevent.h"
#include <iostream>

// 定时器回调函数
void on_timer() {
    std::cout << "Timer triggered!" << std::endl;
}

int main() {
    // 创建服务器，不指定 TCP 端口
    moon::server timer_server(-1);

    // 添加一个周期性定时器，每 1000 毫秒触发一次
    timer_server.add_timeev(1000, true, on_timer);

    // 启动服务器
    timer_server.start();

    return 0;
}
```

**说明 (Explanation):**

1. **创建服务器实例 (Create Server Instance):**

   ```cpp
   moon::server timer_server(-1);
   ```

   创建一个 `server` 对象，不启用 TCP 服务。

2. **添加定时器事件 (Add Timer Event):**

   ```cpp
   timer_server.add_timeev(1000, true, on_timer);
   ```

   添加一个定时器事件，设置超时时间为 `1000` 毫秒（1 秒），`true` 表示周期性触发。

3. **启动服务器 (Start Server):**

   ```cpp
   timer_server.start();
   ```

   启动服务器，开始事件循环，定时器开始工作。

---

### 信号处理示例 (Signal Handling Example)

**描述 (Description):**

使用信号事件处理类，监听并响应特定的 UNIX 信号，如 `SIGINT`（Ctrl+C）。

**代码示例 (Code Example):**

```cpp
#include "server.h"
#include "signalevent.h"
#include <iostream>
#include <vector>
#include <csignal>

// 信号处理回调函数
void on_signal(int signo) {
    std::cout << "Received signal: " << signo << std::endl;
    // 可以在此执行清理操作或优雅退出
}

int main() {
    // 创建服务器，不指定 TCP 端口
    moon::server signal_server(-1);

    // 添加信号事件，监听 SIGINT 和 SIGTERM
    std::vector<int> signals = {SIGINT, SIGTERM};
    signal_server.add_sev(signals, on_signal);

    // 启动服务器
    signal_server.start();

    return 0;
}
```

**说明 (Explanation):**

1. **创建服务器实例 (Create Server Instance):**

   ```cpp
   moon::server signal_server(-1);
   ```

   创建一个 `server` 对象，不启用 TCP 服务。

2. **添加信号事件 (Add Signal Event):**

   ```cpp
   std::vector<int> signals = {SIGINT, SIGTERM};
   signal_server.add_sev(signals, on_signal);
   ```

   添加一个信号事件，监听 `SIGINT` 和 `SIGTERM` 信号，设置回调函数。

3. **启动服务器 (Start Server):**

   ```cpp
   signal_server.start();
   ```

   启动服务器，开始事件循环，信号处理开始工作。

---

### `ringbuff` 使用示例(RingBuff Usage Example)

**代码示例(Code Example)**：

```cpp
#include <moonnet/moonnet.h> //可以只引用总头文件/You can only reference the header file
//#include <moonnet/ringbuff.h>
#include <iostream>
#include <string>
#include <vector>

int main() {
    moon::ringbuff<std::string> buffer(32); // 初始化一个环形缓冲区，容量调整为最近的二次幂

    // 向环形缓冲区中推送元素，使用 push 和 push_move
    buffer.push("你好");
    buffer.push_move(std::string("世界"));

    // 从环形缓冲区中弹出元素
    std::string data;
    if (buffer.pop(data)) {
        std::cout << "弹出: " << data << std::endl;
    }

    // 检查缓冲区大小和容量
    std::cout << "当前缓冲区大小: " << buffer.size() << std::endl;
    std::cout << "缓冲区容量: " << buffer.capacity() << std::endl;

    // 检查缓冲区是否为空或已满
    std::cout << "缓冲区是否为空? " << (buffer.empty() ? "是" : "否") << std::endl;
    std::cout << "缓冲区是否已满? " << (buffer.full() ? "是" : "否") << std::endl;

    // 使用 swap_to_vector 导出缓冲区内容
    std::vector<std::string> vec;
    buffer.swap_to_vector(vec);

    // 显示导出的数据
    for (auto &str : vec) {
        std::cout << "导出: " << str << std::endl;
    }

    return 0;
}
```

---

### `lfthreadpool` 使用示例(LFThreadpool Usage Example)

**代码示例(Code Example)**：

```cpp
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
```



---

## 错误处理 (Error Handling)

MoonNet 使用标准的 POSIX 错误处理机制。当发生错误时，库将调用 `perr_exit` 函数打印错误消息并终止程序。用户可以根据需要自定义错误处理逻辑，例如修改回调函数以处理特定的错误情况。

**示例 (Example):**

在 `wrap.cpp` 中的 `perr_exit` 函数：

```cpp
void perr_exit(const char *s) {
    perror(s);
    exit(1);
}
```

**自定义错误处理 (Custom Error Handling):**

如果希望在发生错误时不直接退出程序，可以修改 `perr_exit` 函数或在回调函数中添加错误处理逻辑。

**示例 (Example):**

在回调函数中处理错误而不是退出：

```cpp
void on_event() {
    std::cerr << "An error occurred on the connection." << std::endl;
    // 执行清理操作或尝试恢复
}
```

---

## 常见问题 (FAQs)

### 1. 如何同时启用 TCP 和 UDP 服务？

**回答 (Answer):**

创建一个 `server` 实例，启用 TCP 服务并添加 UDP 事件。

**示例 (Example):**

```cpp
#include "server.h"
#include "bfevent.h"
#include "udpevent.h"
#include <iostream>

// TCP 读回调函数
void on_tcp_read(moon::bfevent* bev) {
    std::string data = bev->receive();
    if (!data.empty()) {
        std::cout << "TCP Received: " << data << std::endl;
        // 回显数据
        bev->sendout(data);
    }
}

// UDP 接收回调函数
void on_udp_receive(const sockaddr_in& addr, moon::udpevent* uev) {
    std::string data = uev->receive();
    if (!data.empty()) {
        std::cout << "UDP Received from " << inet_ntoa(addr.sin_addr)
                  << ":" << ntohs(addr.sin_port) << " - " << data << std::endl;

        // 发送回复
        std::string reply = "Echo: " + data;
        uev->send_to(reply, addr);
    }
}

int main() {
    // 创建服务器，监听 TCP 端口 8080
    moon::server network_server(8080);

    // 设置 TCP 读事件的回调函数
    network_server.set_tcpcb(on_tcp_read, nullptr, nullptr);

    // 添加 UDP 事件，监听端口 9090
    network_server.add_udpev(9090, on_udp_receive, nullptr);

    // 初始化线程池
    network_server.init_pool();

    // 启动服务器
    network_server.start();

    return 0;
}
```

---

### 2. 如何优雅地关闭服务器？

**回答 (Answer):**

调用 `stop()` 方法来中断事件循环，并确保所有资源被正确释放。

**示例 (Example):**

```cpp
#include "server.h"
#include "signalevent.h"
#include <iostream>
#include <csignal>

moon::server* global_server = nullptr;

// 信号处理回调
void on_signal(int signo) {
    std::cout << "Received signal: " << signo << ", stopping server..." << std::endl;
    if (global_server) {
        global_server->stop();
    }
}

int main() {
    // 创建服务器，监听 TCP 端口 8080
    moon::server network_server(8080);
    global_server = &network_server;

    // 设置 TCP 回调函数（省略）

    // 添加信号处理，监听 SIGINT
    network_server.add_sev(SIGINT, on_signal);

    // 初始化线程池
    network_server.init_pool();

    // 启动服务器
    network_server.start();

    std::cout << "Server stopped gracefully." << std::endl;
    return 0;
}
```

---

### 3. 如何处理多线程中的数据竞争问题？

**回答 (Answer):**

MoonNet 内部通过线程池和事件循环的设计，尽量避免了数据竞争问题。各个事件循环运行在独立的线程中，用户在回调函数中处理数据时，应确保对共享资源的访问是线程安全的，可以使用互斥锁（`std::mutex`）等同步机制。

**示例 (Example):**

```cpp
#include "server.h"
#include "bfevent.h"
#include <iostream>
#include <mutex>

// 共享资源
std::mutex data_mutex;
int shared_data = 0;

// TCP 读回调函数
void on_tcp_read(moon::bfevent* bev) {
    std::string data = bev->receive();
    if (!data.empty()) {
        std::lock_guard<std::mutex> lock(data_mutex);
        shared_data += data.size();
        std::cout << "Shared Data Size: " << shared_data << std::endl;
    }
}

int main() {
    // 创建服务器，监听 TCP 端口 8080
    moon::server network_server(8080);

    // 设置缓冲事件的回调函数
    network_server.set_tcpcb(on_tcp_read, nullptr, nullptr);

    // 初始化线程池
    network_server.init_pool();

    // 启动服务器
    network_server.start();

    return 0;
}
```

---

## 结语 (Conclusion)

MoonNet 提供了一个高效且易用的网络编程框架，适用于构建各种类型的网络应用程序。通过其模块化设计和灵活的接口，开发者可以快速实现高性能的网络服务。希望本文档能够帮助您更好地理解和使用 MoonNet 网络库。

如有任何问题或建议，欢迎联系作者或提交问题反馈。

---

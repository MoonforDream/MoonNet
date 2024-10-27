# MoonNet Guide

---

## Overview

MoonNet is a lightweight, high-performance, event-driven network library based on C++. It aims to simplify the development of network applications, supporting both TCP and UDP protocols. It provides an efficient event loop mechanism for handling asynchronous I/O operations. With its modular design, MoonNet offers flexible interfaces that allow developers to quickly build high-performance network services.

---

## Table of Contents

1. [Module Overview](#module-overview)
2. [Classes and Interfaces](#classes-and-interfaces)
   - [base_event](#base_event)
   - [event](#event)
   - [eventloop](#eventloop)
   - [loopthread](#loopthread)
   - [looptpool](#looptpool)
   - [Threadpool](#threadpool)
   - [buffer](#buffer)
   - [bfevent](#bfevent)
   - [udpevent](#udpevent)
   - [timerevent](#timerevent)
   - [signalevent](#signalevent)
   - [acceptor](#acceptor)
   - [server](#server)
   - [wrap](#wrap)
3. [Usage Examples](#usage-examples)
   - [TCP Server Example](#tcp-server-example)
   - [UDP Server Example](#udp-server-example)
   - [Timer Example](#timer-example)
   - [Signal Handling Example](#signal-handling-example)
4. [Error Handling](#error-handling)
5. [Frequently Asked Questions (FAQs)](#frequently-asked-questions-faqs)
6. [Conclusion](#conclusion)

---

## Module Overview

The core modules of MoonNet include:

- **Event Loop (`eventloop`)**: Manages the registration, deletion, and distribution of events, implementing the core Reactor model.
- **Event (`event`)**: Represents an event on a file descriptor, encapsulating event callbacks and trigger mechanisms.
- **Buffer (`buffer`)**: Handles data buffering for non-blocking I/O, implementing data caching and processing.
- **Buffered Event (`bfevent`)**: A buffer-based event handling class, encapsulating read/write buffers and callbacks.
- **UDP Event (`udpevent`)**: Handles the sending and receiving of UDP packets.
- **Timer Event (`timerevent`)**: Provides timer functionality, supporting one-time and periodic timers.
- **Signal Event (`signalevent`)**: Handles UNIX signals, integrating signal events into the event loop.
- **Acceptor (`acceptor`)**: Listens on TCP ports and accepts new connections.
- **Server (`server`)**: Encapsulates TCP and UDP server functionalities, managing connections, events, and thread pools.

---

## Classes and Interfaces

### `base_event`

**Description:**

`base_event` is the base class for all event types, defining the basic interfaces and behaviors of an event.

**Interface:**

```cpp
namespace moon {

class eventloop;

class base_event {
public:
    virtual ~base_event(){}
    virtual eventloop* getloop() const = 0;
    virtual void close() = 0;
    virtual void disable_cb() = 0;
};

}
```

**Function Descriptions:**

- `virtual eventloop* getloop() const = 0;`  
  Retrieves the associated event loop object.

- `virtual void close() = 0;`  
  Closes the event and releases related resources.

- `virtual void disable_cb() = 0;`  
  Disables the event's callback functions to prevent it from being triggered again.

---

### `event`

**Description:**

The `event` class represents an event on a file descriptor, encapsulating the event type, callback functions, and trigger mechanisms.

**Interface:**

```cpp
namespace moon {

class eventloop;

// Event class
class event : public base_event {
public:
    using Callback = std::function<void()>;

    event(eventloop* base, int fd, uint32_t events);
    ~event();

    int getfd() const;                // Gets the file descriptor
    uint32_t getevents() const;       // Gets the event types being listened to
    eventloop* getloop() const override;

    void setcb(const Callback& rcb, const Callback& wcb, const Callback& ecb); // Sets the callback functions
    void setrcb(const Callback& rcb); // Sets the read event callback
    void setwcb(const Callback& wcb); // Sets the write event callback
    void setecb(const Callback& ecb); // Sets the error event callback

    void setrevents(const uint32_t revents); // Sets the triggered event types
    void enable_events(uint32_t op);  // Enables specific event types
    void disable_events(uint32_t op); // Disables specific event types
    void update_ep();                 // Updates the event listening
    void handle_cb();                 // Handles the event callback

    bool readable();                  // Checks if the event is readable
    bool writeable();                 // Checks if the event is writable
    void enable_read();               // Enables read events
    void disable_read();              // Disables read events
    void enable_write();              // Enables write events
    void disable_write();             // Disables write events
    void enable_ET();                 // Enables edge-triggered mode
    void disable_ET();                // Disables edge-triggered mode

    void reset_events();              // Resets the event types
    void del_listen();                // Removes the event listening
    void enable_listen();             // Enables event listening
    void disable_cb() override;       // Disables the callback functions
    void close() override;            // Closes the event

private:
    eventloop* loop_;
    int fd_;
    uint32_t events_;   // Events being listened to
    uint32_t revents_;  // Triggered events
    Callback readcb_;   // Read event callback
    Callback writecb_;  // Write event callback
    Callback eventcb_;  // Error event callback
};

}
```

**Function Descriptions:**

- `event(eventloop* base, int fd, uint32_t events);`  
  Constructor that initializes the event object.

- `~event();`  
  Destructor that releases resources.

- `int getfd() const;`  
  Gets the file descriptor associated with the event.

- `uint32_t getevents() const;`  
  Gets the event types being listened to.

- `eventloop* getloop() const override;`  
  Retrieves the associated event loop object.

- `void setcb(const Callback& rcb, const Callback& wcb, const Callback& ecb);`  
  Sets the read, write, and error event callback functions.

- `void setrcb(const Callback& rcb);`  
  Sets the read event callback function.

- `void setwcb(const Callback& wcb);`  
  Sets the write event callback function.

- `void setecb(const Callback& ecb);`  
  Sets the error event callback function.

- `void setrevents(const uint32_t revents);`  
  Sets the triggered event types.

- `void enable_events(uint32_t op);`  
  Enables specific event types.

- `void disable_events(uint32_t op);`  
  Disables specific event types.

- `void update_ep();`  
  Updates the event's status in epoll.

- `void handle_cb();`  
  Handles the event by calling the appropriate callback functions.

- `bool readable();`  
  Checks if the event is readable.

- `bool writeable();`  
  Checks if the event is writable.

- `void enable_read();`  
  Enables read event listening.

- `void disable_read();`  
  Disables read event listening.

- `void enable_write();`  
  Enables write event listening.

- `void disable_write();`  
  Disables write event listening.

- `void enable_ET();`  
  Enables edge-triggered mode.

- `void disable_ET();`  
  Disables edge-triggered mode.

- `void reset_events();`  
  Resets the event types.

- `void del_listen();`  
  Removes the event listening.

- `void enable_listen();`  
  Enables event listening.

- `void disable_cb() override;`  
  Disables the event's callback functions.

- `void close() override;`  
  Closes the event.

---

### `eventloop`

**Description:**

The `eventloop` class is the core of the event loop, implementing the Reactor model and managing the registration, deletion, and distribution of all events.

**Interface:**

```cpp
namespace moon {

class base_event;
class event;
class loopthread;

// Event loop class
class eventloop {
public:
    using Callback = std::function<void()>;

    eventloop(loopthread* base = nullptr, int timeout = -1);
    ~eventloop();

    loopthread* getbaseloop();
    int getefd() const;
    int getevfd() const;
    int getload() const;

    // Event control functions
    void add_event(event* event);
    void del_event(event* event);
    void mod_event(event* event);

    void loop();          // Starts the event loop
    void loopbreak();     // Stops the event loop
    void getallev(std::list<event*>& list);

    void create_eventfd();     // Creates a notification file descriptor
    void read_eventfd();
    void write_eventfd();

    void add_pending_del(base_event* ev); // Adds an event to the pending deletion queue

private:
    void updateload(int n); // Updates the load

private:
    int epfd_;                         // epoll file descriptor
    int eventfd_;                      // Event notification file descriptor
    int timeout_;                      // epoll timeout
    std::atomic<int> load_;            // Load (number of active events)
    std::atomic<bool> shutdown_;       // Whether to shut down the event loop
    std::list<event*> evlist_;         // List of events
    std::vector<epoll_event> events_;  // Array of epoll events
    std::vector<base_event*> delque_;  // Queue of events pending deletion
    loopthread* baseloop_;             // Associated thread
};

}
```

**Function Descriptions:**

- `eventloop(loopthread* base = nullptr, int timeout = -1);`  
  Constructor that initializes the event loop object.

- `~eventloop();`  
  Destructor that releases resources.

- `loopthread* getbaseloop();`  
  Gets the associated thread object.

- `int getefd() const;`  
  Gets the epoll file descriptor.

- `int getevfd() const;`  
  Gets the event notification file descriptor.

- `int getload() const;`  
  Gets the current load (number of active events) of the event loop.

- `void add_event(event* event);`  
  Adds an event to be listened to by epoll.

- `void del_event(event* event);`  
  Removes an event from epoll.

- `void mod_event(event* event);`  
  Modifies the event's listening types.

- `void loop();`  
  Starts the event loop and handles events.

- `void loopbreak();`  
  Stops the event loop.

- `void getallev(std::list<event*>& list);`  
  Retrieves all events in the list.

- `void create_eventfd();`  
  Creates an event notification file descriptor.

- `void read_eventfd();`  
  Reads from the event notification file descriptor to handle loop termination signals.

- `void write_eventfd();`  
  Writes to the event notification file descriptor to notify the event loop to terminate.

- `void add_pending_del(base_event* ev);`  
  Adds an event to the pending deletion queue.

---

### `loopthread`

**Description:**

The `loopthread` class encapsulates an event loop thread used to run an `eventloop`.

**Interface:**

```cpp
namespace moon {

class eventloop;

class loopthread {
public:
    loopthread(int timeout = -1);
    ~loopthread();

    eventloop* getloop(); // Gets the event loop object

private:
    void _init_(); // Initializes the event loop

private:
    eventloop* loop_;
    std::thread t_;
    std::mutex mx_;
    std::condition_variable cv_;
    int timeout_;
};

}
```

**Function Descriptions:**

- `loopthread(int timeout = -1);`  
  Constructor that initializes the thread and creates the event loop.

- `~loopthread();`  
  Destructor that terminates the thread and releases resources.

- `eventloop* getloop();`  
  Gets the event loop object.

- `void _init_();`  
  Internal function that initializes the event loop and starts looping.

---

### `looptpool`

**Description:**

The `looptpool` class manages a group of event loop threads (`loopthread`), implementing thread pool functionality and providing static/dynamic load balancing.

**Interface:**

```cpp
namespace moon {

class eventloop;
class loopthread;

class looptpool {
public:
    looptpool(eventloop* base, bool dispath = false); // By default, dynamic load balancing is disabled
    ~looptpool();

    void create_pool(int timeout = -1);                  // Creates the thread pool
    void create_pool(int n, int timeout);                // Creates the thread pool with specified number of threads
    void create_pool_noadjust(int n, int timeout);       // Creates the thread pool without dynamic adjustment

    eventloop* ev_dispatch();    // Dispatches events to event loops in the thread pool
    void delloop_dispatch();     // Deletes a sub-reactor and redistributes events
    void addloop();              // Adds a new event loop thread
    void adjust_task();          // Management thread task, dynamically adjusting sub-reactors
    int getscale();              // Gets the average load
    void enable_adjust();        // Enables dynamic load balancing
    void stop();                 // Stops the thread pool

    // Disables copy and assignment operations
    looptpool(const looptpool&) = delete;
    looptpool& operator=(const looptpool&) = delete;

private:
    void init_pool(int timeout = -1); // Initializes the thread pool
    eventloop* getminload();          // Gets the event loop with the minimum load
    int getmaxidx();                  // Gets the index of the event loop with the maximum load

private:
    eventloop* baseloop_;            // Main event loop
    std::thread manager_;            // Management thread
    std::vector<eventloop*> loadvec_;// List of event loops
    int next_;                       // Round-robin index
    int t_num_;                      // Number of threads
    int timeout_;                    // Timeout duration
    int max_tnum_;                   // Maximum number of threads
    int min_tnum_;                   // Minimum number of threads
    bool dispath_;                   // Whether dynamic dispatch is enabled
    int coolsec_;                    // Cool-down time
    int timesec_;                    // Dispatch interval
    int scale_max_;                  // Maximum load scale
    int scale_min_;                  // Minimum load scale
};

}
```

**Function Descriptions:**

- `looptpool(eventloop* base, bool dispath = false);`  
  Constructor that initializes the thread pool and specifies whether to enable dynamic load balancing.

- `~looptpool();`  
  Destructor that releases resources.

- `void create_pool(int timeout = -1);`  
  Creates the thread pool with the default number of threads.

- `void create_pool(int n, int timeout);`  
  Creates the thread pool with a specified number of threads.

- `void create_pool_noadjust(int n, int timeout);`  
  Creates the thread pool without dynamic adjustment.

- `eventloop* ev_dispatch();`  
  Dispatches events to event loops in the thread pool.

- `void delloop_dispatch();`  
  Deletes the event loop with the maximum load and redistributes its events.

- `void addloop();`  
  Adds a new event loop thread.

- `void adjust_task();`  
  Management thread task for dynamically adjusting the number of threads.

- `int getscale();`  
  Gets the average load scale.

- `void enable_adjust();`  
  Enables dynamic load balancing.

- `void stop();`  
  Stops the thread pool and terminates all event loops.

---

### `Threadpool`

**Description:**

The `Threadpool` class implements a general-purpose thread pool for executing arbitrary task functions.

**Interface:**

```cpp
namespace moon {

class Threadpool {
public:
    Threadpool(int num);
    ~Threadpool();

    // Adds a task to the thread pool
    template<typename _Fn, typename... _Args>
    void add_task(_Fn&& fn, _Args&&... args);

private:
    void init();          // Initializes the thread pool
    void t_shutdown();    // Shuts down the thread pool
    void t_task();        // Task thread entry function
    void adjust_task();   // Management thread entry function

private:
    std::thread adjust_thr;                    // Management thread
    std::vector<std::thread> threads;          // Worker threads
    std::queue<std::function<void()>> tasks;   // Task queue
    std::mutex mx;                             // Mutex
    std::condition_variable task_cv;           // Condition variable
    int min_thr_num;                           // Minimum number of threads
    int max_thr_num;                           // Maximum number of threads
    std::atomic<int> run_num;                  // Number of threads executing tasks
    std::atomic<int> live_num;                 // Number of live threads
    std::atomic<int> exit_num;                 // Number of threads to terminate
    bool shutdown;                             // Whether the thread pool is shut down
};

}
```

**Function Descriptions:**

- `Threadpool(int num);`  
  Constructor that initializes the thread pool with a specified minimum number of threads.

- `~Threadpool();`  
  Destructor that shuts down the thread pool.

- `void add_task(_Fn&& fn, _Args&&... args);`  
  Adds a task to the thread pool, specifying the task function and its arguments.

- `void init();`  
  Initializes the thread pool, creating worker and management threads.

- `void t_shutdown();`  
  Shuts down the thread pool, terminating all threads.

- `void t_task();`  
  Entry function for worker threads to execute tasks.

- `void adjust_task();`  
  Entry function for the management thread to dynamically adjust the number of threads.

---

### `buffer`

**Description:**

The `buffer` class implements an auto-expanding buffer for handling data caching and operations in non-blocking I/O.

**Interface:**

```cpp
namespace moon {

class buffer {
public:
    buffer();
    ~buffer();

    void append(const char* data, size_t len); // Appends data to the buffer
    size_t remove(char* data, size_t len);     // Reads data from the buffer
    std::string remove(size_t len);            // Reads data from the buffer and returns a string
    void retrieve(size_t len);                 // Moves the read pointer without copying data
    size_t readbytes() const;                  // Gets the size of readable data
    size_t writebytes() const;                 // Gets the size of writable space
    const char* peek() const;                  // Gets the data at the current read pointer
    void reset();                              // Resets the buffer
    ssize_t readiov(int fd, int& errnum);      // Reads data from a file descriptor into the buffer

private:
    void able_wirte(size_t len);               // Ensures there is enough writable space

private:
    std::vector<char> buffer_;
    uint64_t reader_;    // Read pointer position
    uint64_t writer_;    // Write pointer position
};

}
```

**Function Descriptions:**

- `buffer();`  
  Constructor that initializes the buffer.

- `~buffer();`  
  Destructor that releases resources.

- `void append(const char* data, size_t len);`  
  Appends data to the buffer.

- `size_t remove(char* data, size_t len);`  
  Reads data from the buffer into a specified memory location.

- `std::string remove(size_t len);`  
  Reads data from the buffer and returns it as a string.

- `void retrieve(size_t len);`  
  Moves the read pointer forward, marking data as read without copying.

- `size_t readbytes() const;`  
  Gets the number of bytes available for reading.

- `size_t writebytes() const;`  
  Gets the number of bytes available for writing.

- `const char* peek() const;`  
  Gets a pointer to the data at the current read position.

- `void reset();`  
  Resets the buffer, clearing all data.

- `ssize_t readiov(int fd, int& errnum);`  
  Reads data from a file descriptor into the buffer using scatter/gather I/O.

---

### `bfevent`

**Description:**

The `bfevent` class is a buffer-based event handling class, encapsulating read/write buffers and callbacks, handling data transmission for TCP connections.

**Interface:**

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

    void setcb(const RCallback& rcb, const Callback& wcb, const Callback& ecb); // Sets the callback functions
    void setrcb(const RCallback& rcb);
    void setwcb(const Callback& wcb);
    void setecb(const Callback& ecb);
    RCallback getrcb();
    Callback getwcb();
    Callback getecb();

    void update_ep();                // Updates the event listening
    void del_listen();               // Cancels event listening
    void enable_listen();            // Enables event listening

    void sendout(const char* data, size_t len);   // Sends data
    void sendout(const std::string& data);        // Sends data
    size_t receive(char* data, size_t len);       // Receives data into specified memory
    std::string receive(size_t len);              // Receives a specified length of data
    std::string receive();                        // Receives all available data

    void enable_events(uint32_t op);  // Enables specific events
    void disable_events(uint32_t op); // Disables specific events
    void enable_read();               // Enables read events
    void disable_read();              // Disables read events
    void enable_write();              // Enables write events
    void disable_write();             // Disables write events
    void enable_ET();                 // Enables edge-triggered mode
    void disable_ET();                // Disables edge-triggered mode
    void disable_cb() override;       // Disables callback functions

    void close() override;            // Closes the event

private:
    void close_event();               // Internal function to close the event
    void handle_read();               // Handles read events
    void handle_write();              // Handles write events
    void handle_event();              // Handles error events

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

**Function Descriptions:**

- `bfevent(eventloop* base, int fd, uint32_t events);`  
  Constructor that initializes the buffered event object.

- `~bfevent();`  
  Destructor that closes the event and releases resources.

- `int getfd() const;`  
  Gets the file descriptor.

- `eventloop* getloop() const override;`  
  Retrieves the associated event loop.

- `buffer* getinbuff();`  
  Gets the input buffer.

- `buffer* getoutbuff();`  
  Gets the output buffer.

- `bool writeable() const;`  
  Checks if the event is writable.

- `void setcb(const RCallback& rcb, const Callback& wcb, const Callback& ecb);`  
  Sets the read, write, and error event callback functions.

- `void update_ep();`  
  Updates the event listening.

- `void del_listen();`  
  Cancels event listening.

- `void enable_listen();`  
  Enables event listening.

- `void sendout(const char* data, size_t len);`  
  Sends data.

- `void sendout(const std::string& data);`  
  Sends data.

- `size_t receive(char* data, size_t len);`  
  Receives data into specified memory.

- `std::string receive(size_t len);`  
  Receives a specified length of data.

- `std::string receive();`  
  Receives all available data.

- `void enable_events(uint32_t op);`  
  Enables specific events.

- `void disable_events(uint32_t op);`  
  Disables specific events.

- `void enable_read();`  
  Enables read events.

- `void disable_read();`  
  Disables read events.

- `void enable_write();`  
  Enables write events.

- `void disable_write();`  
  Disables write events.

- `void enable_ET();`  
  Enables edge-triggered mode.

- `void disable_ET();`  
  Disables edge-triggered mode.

- `void disable_cb() override;`  
  Disables callback functions.

- `void close() override;`  
  Closes the event.

---

### `udpevent`

**Description:**

The `udpevent` class handles the sending and receiving of data over the UDP protocol, supporting non-blocking UDP communication.

**Interface:**

```cpp
namespace moon {

class eventloop;
class event;

// UDP event handling class
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
    void start();       // Starts listening
    void stop();        // Stops listening
    void update_ep();   // Updates event listening

    size_t receive(char* data, size_t len);  // Receives data into specified memory
    std::string receive(size_t len);         // Receives a specified length of data
    std::string receive();                   // Receives all available data

    void send_to(const std::string& data, const sockaddr_in& addr); // Sends data to a specified address

    void enable_read();
    void disable_read();
    void enable_ET();
    void disable_ET();
    void disable_cb() override;
    RCallback getrcb();
    Callback getecb();
    void close() override; // Closes the event

private:
    void handle_receive(); // Handles receive events

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

**Function Descriptions:**

- `udpevent(eventloop* base, int port);`  
  Constructor that initializes the UDP event object.

- `~udpevent();`  
  Destructor that closes the event and releases resources.

- `buffer* getinbuff();`  
  Gets the input buffer.

- `eventloop* getloop() const override;`  
  Retrieves the associated event loop.

- `void setcb(const RCallback& rcb, const Callback& ecb);`  
  Sets the receive and error event callback functions.

- `void init_sock(int port);`  
  Initializes the UDP socket.

- `void start();`  
  Starts listening for UDP packets.

- `void stop();`  
  Stops listening.

- `void update_ep();`  
  Updates event listening.

- `size_t receive(char* data, size_t len);`  
  Receives data from the buffer.

- `std::string receive(size_t len);`  
  Receives a specified length of data from the buffer.

- `std::string receive();`  
  Receives all available data from the buffer.

- `void send_to(const std::string& data, const sockaddr_in& addr);`  
  Sends data to a specified address.

- `void enable_read();`  
  Enables read events.

- `void disable_read();`  
  Disables read events.

- `void enable_ET();`  
  Enables edge-triggered mode.

- `void disable_ET();`  
  Disables edge-triggered mode.

- `void disable_cb() override;`  
  Disables callback functions.

- `void close() override;`  
  Closes the event.

---

### `timerevent`

**Description:**

The `timerevent` class implements timer functionality, supporting both one-time and periodic timers for executing tasks at intervals.

**Interface:**

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
    void start();
    void stop();
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

**Function Descriptions:**

- `timerevent(eventloop* loop, int timeout_ms, bool periodic);`  
  Constructor that initializes the timer event.

- `~timerevent();`  
  Destructor that closes the timer and releases resources.

- `int getfd() const;`  
  Gets the timer file descriptor.

- `eventloop* getloop() const override;`  
  Retrieves the associated event loop.

- `void setcb(const Callback& cb);`  
  Sets the timer callback function.

- `void start();`  
  Starts the timer.

- `void stop();`  
  Stops the timer.

- `void close() override;`  
  Closes the timer event.

- `void disable_cb() override;`  
  Disables callback functions.

---

### `signalevent`

**Description:**

The `signalevent` class handles UNIX signals, integrating signal events into the event loop using a pipe mechanism.

**Interface:**

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

    void enable_listen();
    void del_listen();
    void disable_cb() override;
    void close() override;
    Callback getcb();

private:
    static void handle_signal(int signo);
    void handle_read();

private:
    eventloop* loop_;
    int pipe_fd_[2]; // Pipe read and write ends
    event* ev_;
    Callback cb_;
    static signalevent* sigev_; // Singleton instance
};

}
```

**Function Descriptions:**

- `signalevent(eventloop* base);`  
  Constructor that initializes the signal event object.

- `~signalevent();`  
  Destructor that closes the event and releases resources.

- `void add_signal(int signo);`  
  Adds a single signal to listen for.

- `void add_signal(const std::vector<int>& signals);`  
  Adds multiple signals to listen for.

- `void setcb(const Callback& cb);`  
  Sets the signal handling callback function.

- `void enable_listen();`  
  Enables signal listening.

- `void del_listen();`  
  Stops signal listening.

- `void disable_cb() override;`  
  Disables callback functions.

- `void close() override;`  
  Closes the signal event.

---

### `acceptor`

**Description:**

The `acceptor` class is responsible for listening on a specified port, accepting new TCP connections, and handing them off to a callback function.

**Interface:**

```cpp
namespace moon {

class eventloop;
class event;

// Acceptor class
class acceptor {
public:
    using Callback = std::function<void(int)>;

    acceptor(int port, eventloop* base);
    ~acceptor();

    void listen();               // Starts listening
    void stop();                 // Stops listening
    void init_sock(int port);    // Initializes the listening socket
    void setcb(const Callback& accept_cb); // Sets the callback function
    void handle_accept();        // Handles new connections

private:
    int lfd_;           // Listening socket
    eventloop* loop_;   // Event loop
    event* ev_;         // Event object
    Callback cb_;       // Callback function
    bool shutdown_;     // Whether it has been shut down
};

}
```

**Function Descriptions:**

- `acceptor(int port, eventloop* base);`  
  Constructor that initializes the acceptor object.

- `~acceptor();`  
  Destructor that stops listening and releases resources.

- `void listen();`  
  Starts listening.

- `void stop();`  
  Stops listening.

- `void init_sock(int port);`  
  Initializes the listening socket.

- `void setcb(const Callback& accept_cb);`  
  Sets the callback function for new connections.

- `void handle_accept();`  
  Handles new connection events.

---

### `server`

**Description:**

The `server` class encapsulates TCP and UDP server functionalities, managing connections, events, and thread pools.

**Interface:**

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

    void start();                         // Starts the server
    void stop();                          // Stops the server
    void init_pool(int timeout = -1);     // Initializes the thread pool
    void init_pool(int tnum, int timeout);// Initializes the thread pool with a specified number of threads
    void init_pool_noadjust(int tnum, int timeout); // Initializes the thread pool without dynamic adjustment
    void enable_tcp(int port);            // Enables TCP service
    void enable_tcp_accept();             // Enables TCP connection listening
    void disable_tcp_accept();            // Disables TCP connection listening
    eventloop* getloop();                 // Gets the main event loop
    eventloop* dispatch();                // Dispatches events

    // Sets the callback functions for TCP connections
    void set_tcpcb(const RCallback& rcb, const Callback& wcb, const Callback& ecb);

    // Event operations
    void add_ev(event* ev);
    void del_ev(event* ev);
    void mod_ev(event* ev);
    void add_bev(bfevent* bev);
    void del_bev(bfevent* bev);
    void mod_bev(bfevent* bev);
    void add_udpev(udpevent* uev);
    udpevent* add_udpev(int port, const UCallback& rcb, const Callback& ecb);
    void del_udpev(udpevent* uev);
    void mod_udpev(udpevent* uev);
    void add_sev(signalevent* sev);
    signalevent* add_sev(int signo, const SCallback& cb);
    signalevent* add_sev(const std::vector<int>& signals, const SCallback& cb);
    void del_sev(signalevent* sev);
    void add_timeev(timerevent* tev);
    timerevent* add_timeev(int timeout_ms, bool periodic, const Callback& cb);
    void del_timeev(timerevent* tev);

private:
    void acceptcb_(int fd); // Handles new connections

    void tcp_eventcb_(bfevent* bev); // Handles TCP error events

    void handle_close(base_event* ev); // Handles event closures

private:
    std::mutex events_mutex_;
    eventloop base_;             // Main event loop
    looptpool pool_;             // Thread pool
    acceptor acceptor_;          // Acceptor
    int port_;                   // Port number
    bool tcp_enable_;            // Whether TCP is enabled
    std::list<base_event*> events_; // List of events

    // TCP connection callback functions
    RCallback readcb_;
    Callback writecb_;
    Callback eventcb_;
};

}
```

**Function Descriptions:**

- `server(int port = -1);`  
  Constructor that initializes the server object.

- `~server();`  
  Destructor that shuts down the server and releases resources.

- `void start();`  
  Starts the server and begins the event loop.

- `void stop();`  
  Stops the server and terminates the event loop.

- `void init_pool(int timeout = -1);`  
  Initializes the thread pool.

- `void enable_tcp(int port);`  
  Enables TCP service.

- `void enable_tcp_accept();`  
  Enables TCP connection listening.

- `void disable_tcp_accept();`  
  Disables TCP connection listening.

- `eventloop* getloop();`  
  Gets the main event loop.

- `eventloop* dispatch();`  
  Dispatches events to the thread pool.

- `void set_tcpcb(const RCallback& rcb, const Callback& wcb, const Callback& ecb);`  
  Sets the callback functions for TCP connections.

- `void add_ev(event* ev);`  
  Adds an event.

- `void del_ev(event* ev);`  
  Deletes an event.

- `void mod_ev(event* ev);`  
  Modifies an event.

- `void add_bev(bfevent* bev);`  
  Adds a buffered event.

- `void del_bev(bfevent* bev);`  
  Deletes a buffered event.

- `void mod_bev(bfevent* bev);`  
  Modifies a buffered event.

- `void add_udpev(udpevent* uev);`  
  Adds a UDP event.

- `udpevent* add_udpev(int port, const UCallback& rcb, const Callback& ecb);`  
  Adds and initializes a UDP event.

- `void del_udpev(udpevent* uev);`  
  Deletes a UDP event.

- `void mod_udpev(udpevent* uev);`  
  Modifies a UDP event.

- `void add_sev(signalevent* sev);`  
  Adds a signal event.

- `signalevent* add_sev(int signo, const SCallback& cb);`  
  Adds and initializes a signal event.

- `void del_sev(signalevent* sev);`  
  Deletes a signal event.

- `timerevent* add_timeev(int timeout_ms, bool periodic, const Callback& cb);`  
  Adds and initializes a timer event.

- `void del_timeev(timerevent* tev);`  
  Deletes a timer event.

---

### `wrap`

**Description:**

The `wrap` module encapsulates cross-platform socket operation functions, providing a unified interface for both Windows and Unix systems.

**Interface:**

```cpp
#ifndef _WRAP_H_
#define _WRAP_H_

#ifdef _WIN32

// Socket encapsulation functions for Windows platform
void setreuse(SOCKET fd);
void setnonblock(SOCKET fd);
void perr_exit(const char* s);
// Other function definitions...

#else

// Socket encapsulation functions for Unix/Linux platform
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
// Other function definitions...

#endif

#endif
```

**Function Descriptions:**

- `void settcpnodelay(int fd);`  
  Sets the socket to no-delay mode (`TCP_NODELAY`).

- `void setreuse(int fd);`  
  Sets the socket's address and port to be reusable.

- `void setnonblock(int fd);`  
  Sets the socket to non-blocking mode.

- `void perr_exit(const char* s);`  
  Prints an error message and exits the program.

- `int Accept(int fd, struct sockaddr* sa, socklen_t* salenptr);`  
  Accepts a new connection.

- `int Bind(int fd, const struct sockaddr* sa, socklen_t salen);`  
  Binds the socket to a specified address and port.

- `int Connect(int fd, const struct sockaddr* sa, socklen_t salen);`  
  Connects to a specified address and port.

- `int Listen(int fd, int backlog);`  
  Starts listening on the socket.

- `int Socket(int family, int type, int protocol);`  
  Creates a new socket.

- `ssize_t Read(int fd, void* ptr, size_t nbytes);`  
  Reads data from the socket.

- `ssize_t Write(int fd, const void* ptr, size_t nbytes);`  
  Writes data to the socket.

- `int Close(int fd);`  
  Closes the socket.

- `ssize_t Readn(int fd, void* vptr, size_t n);`  
  Reads a specified number of bytes from the socket.

- `ssize_t Writen(int fd, const void* vptr, size_t n);`  
  Writes a specified number of bytes to the socket.

- `ssize_t Readline(int fd, void* vptr, size_t maxlen);`  
  Reads a line of data from the socket.

---

## Usage Examples

The following examples demonstrate how to use the MoonNet network library to build TCP and UDP servers, as well as how to use timer and signal handling functionalities.

### TCP Server Example

**Description:**

Creates a simple TCP server that listens on a specified port, accepts client connections, and echoes back received data.

**Code Example:**

```cpp
#include "server.h"
#include "bfevent.h"
#include <iostream>

// Read event callback function
void on_read(moon::bfevent* bev) {
    std::string data = bev->receive();
    if (!data.empty()) {
        std::cout << "Received: " << data << std::endl;
        // Echo the data back
        bev->sendout(data);
    }
}

// Write event callback function
void on_write(){
    // Handle write events if necessary
}

// Other event callback function (e.g., error)
void on_event() {
    std::cout << "An error occurred on the connection." << std::endl;
}

int main() {
    // Create a server listening on port 8080
    moon::server tcp_server(8080);

    // Set the buffered event callback functions
    tcp_server.set_tcpcb(on_read, on_write, on_event);

    // Initialize the thread pool (optional)
    tcp_server.init_pool();

    // Start the server
    tcp_server.start();

    return 0;
}
```

**Explanation:**

1. **Create Server Instance:**

   ```cpp
   moon::server tcp_server(8080);
   ```

   Creates a `server` object that listens on TCP port `8080`.

2. **Set Callback Functions:**

   ```cpp
   tcp_server.set_tcpcb(on_read, on_write, on_event);
   ```

   Sets the buffered event's read, write, and error callback functions.

3. **Initialize Thread Pool:**

   ```cpp
   tcp_server.init_pool();
   ```

   Initializes the thread pool to improve the server's concurrency handling capabilities.

4. **Start Server:**

   ```cpp
   tcp_server.start();
   ```

   Starts the server, begins the event loop, and listens for and handles connections.

---

### UDP Server Example

**Description:**

Creates a simple UDP server that listens on a specified port, receives data packets, and replies to clients.

**Code Example:**

```cpp
#include "server.h"
#include "udpevent.h"
#include <iostream>

// UDP receive callback function
void on_udp_receive(const sockaddr_in& addr, moon::udpevent* uev) {
    std::string data = uev->receive();
    if (!data.empty()) {
        std::cout << "UDP Received from " << inet_ntoa(addr.sin_addr)
                  << ":" << ntohs(addr.sin_port) << " - " << data << std::endl;

        // Send a reply
        std::string reply = "Echo: " + data;
        uev->send_to(reply, addr);
    }
}

int main() {
    // Create a server without specifying a TCP port
    moon::server udp_server(-1);

    // Add a UDP event listening on port 5005
    udp_server.add_udpev(5005, on_udp_receive, nullptr);

    // Start the server
    udp_server.start();

    return 0;
}
```

**Explanation:**

1. **Create Server Instance:**

   ```cpp
   moon::server udp_server(-1);
   ```

   Creates a `server` object without enabling TCP service.

2. **Add UDP Event:**

   ```cpp
   udp_server.add_udpev(5005, on_udp_receive, nullptr);
   ```

   Adds a UDP event that listens on port `5005` and sets the receive callback function.

3. **Start Server:**

   ```cpp
   udp_server.start();
   ```

   Starts the server, begins the event loop, and listens for and handles UDP packets.

---

### Timer Example

**Description:**

Uses a timer to periodically execute a task, such as printing a message every second.

**Code Example:**

```cpp
#include "server.h"
#include "timerevent.h"
#include <iostream>

// Timer callback function
void on_timer() {
    std::cout << "Timer triggered!" << std::endl;
}

int main() {
    // Create a server without specifying a TCP port
    moon::server timer_server(-1);

    // Add a periodic timer that triggers every 1000 milliseconds
    timer_server.add_timeev(1000, true, on_timer);

    // Start the server
    timer_server.start();

    return 0;
}
```

**Explanation:**

1. **Create Server Instance:**

   ```cpp
   moon::server timer_server(-1);
   ```

   Creates a `server` object without enabling TCP service.

2. **Add Timer Event:**

   ```cpp
   timer_server.add_timeev(1000, true, on_timer);
   ```

   Adds a timer event with a timeout of `1000` milliseconds (1 second). The `true` parameter indicates that the timer is periodic.

3. **Start Server:**

   ```cpp
   timer_server.start();
   ```

   Starts the server, begins the event loop, and the timer starts working.

---

### Signal Handling Example

**Description:**

Uses the signal event handling class to listen for and respond to specific UNIX signals, such as `SIGINT` (Ctrl+C).

**Code Example:**

```cpp
#include "server.h"
#include "signalevent.h"
#include <iostream>
#include <vector>
#include <csignal>

// Signal handling callback function
void on_signal(int signo) {
    std::cout << "Received signal: " << signo << std::endl;
    // Perform cleanup operations or graceful exit here
}

int main() {
    // Create a server without specifying a TCP port
    moon::server signal_server(-1);

    // Add signal events to listen for SIGINT and SIGTERM
    std::vector<int> signals = {SIGINT, SIGTERM};
    signal_server.add_sev(signals, on_signal);

    // Start the server
    signal_server.start();

    return 0;
}
```

**Explanation:**

1. **Create Server Instance:**

   ```cpp
   moon::server signal_server(-1);
   ```

   Creates a `server` object without enabling TCP service.

2. **Add Signal Event:**

   ```cpp
   std::vector<int> signals = {SIGINT, SIGTERM};
   signal_server.add_sev(signals, on_signal);
   ```

   Adds a signal event to listen for `SIGINT` and `SIGTERM` signals and sets the callback function.

3. **Start Server:**

   ```cpp
   signal_server.start();
   ```

   Starts the server, begins the event loop, and the signal handling starts working.

---

## Error Handling

MoonNet uses standard POSIX error handling mechanisms. When an error occurs, the library calls the `perr_exit` function to print an error message and terminate the program. Users can customize error handling logic as needed, such as modifying callback functions to handle specific error situations.

**Example:**

In `wrap.cpp`, the `perr_exit` function:

```cpp
void perr_exit(const char *s) {
    perror(s);
    exit(1);
}
```

**Custom Error Handling:**

If you want to avoid directly exiting the program when an error occurs, you can modify the `perr_exit` function or add error handling logic in the callback functions.

**Example:**

Handling errors in the callback function instead of exiting:

```cpp
void on_event() {
    std::cerr << "An error occurred on the connection." << std::endl;
    // Perform cleanup operations or attempt recovery
}
```

---

## Frequently Asked Questions (FAQs)

### 1. How do I enable both TCP and UDP services simultaneously?

**Answer:**

Create a `server` instance, enable TCP service, and add UDP events.

**Example:**

```cpp
#include "server.h"
#include "bfevent.h"
#include "udpevent.h"
#include <iostream>

// TCP read callback function
void on_tcp_read(moon::bfevent* bev) {
    std::string data = bev->receive();
    if (!data.empty()) {
        std::cout << "TCP Received: " << data << std::endl;
        // Echo the data back
        bev->sendout(data);
    }
}

// UDP receive callback function
void on_udp_receive(const sockaddr_in& addr, moon::udpevent* uev) {
    std::string data = uev->receive();
    if (!data.empty()) {
        std::cout << "UDP Received from " << inet_ntoa(addr.sin_addr)
                  << ":" << ntohs(addr.sin_port) << " - " << data << std::endl;

        // Send a reply
        std::string reply = "Echo: " + data;
        uev->send_to(reply, addr);
    }
}

int main() {
    // Create a server listening on TCP port 8080
    moon::server network_server(8080);

    // Set the TCP read event callback function
    network_server.set_tcpcb(on_tcp_read, nullptr, nullptr);

    // Add a UDP event listening on port 9090
    network_server.add_udpev(9090, on_udp_receive, nullptr);

    // Initialize the thread pool
    network_server.init_pool();

    // Start the server
    network_server.start();

    return 0;
}
```

---

### 2. How do I gracefully shut down the server?

**Answer:**

Call the `stop()` method to interrupt the event loop and ensure all resources are properly released.

**Example:**

```cpp
#include "server.h"
#include "signalevent.h"
#include <iostream>
#include <csignal>

moon::server* global_server = nullptr;

// Signal handling callback
void on_signal(int signo) {
    std::cout << "Received signal: " << signo << ", stopping server..." << std::endl;
    if (global_server) {
        global_server->stop();
    }
}

int main() {
    // Create a server listening on TCP port 8080
    moon::server network_server(8080);
    global_server = &network_server;

    // Set TCP callback functions (omitted)

    // Add signal handling for SIGINT
    network_server.add_sev(SIGINT, on_signal);

    // Initialize the thread pool
    network_server.init_pool();

    // Start the server
    network_server.start();

    std::cout << "Server stopped gracefully." << std::endl;
    return 0;
}
```

---

### 3. How do I handle data race issues in multithreading?

**Answer:**

MoonNet minimizes data race issues through its design of thread pools and event loops. Each event loop runs in an independent thread. When processing data in callback functions, users should ensure thread-safe access to shared resources, using synchronization mechanisms like mutexes (`std::mutex`).

**Example:**

```cpp
#include "server.h"
#include "bfevent.h"
#include <iostream>
#include <mutex>

// Shared resource
std::mutex data_mutex;
int shared_data = 0;

// TCP read callback function
void on_tcp_read(moon::bfevent* bev) {
    std::string data = bev->receive();
    if (!data.empty()) {
        std::lock_guard<std::mutex> lock(data_mutex);
        shared_data += data.size();
        std::cout << "Shared Data Size: " << shared_data << std::endl;
    }
}

int main() {
    // Create a server listening on TCP port 8080
    moon::server network_server(8080);

    // Set the buffered event callback functions
    network_server.set_tcpcb(on_tcp_read, nullptr, nullptr);

    // Initialize the thread pool
    network_server.init_pool();

    // Start the server
    network_server.start();

    return 0;
}
```

---

## Conclusion

MoonNet provides an efficient and easy-to-use network programming framework suitable for building various types of network applications. With its modular design and flexible interfaces, developers can quickly implement high-performance network services. We hope this documentation helps you better understand and use the MoonNet network library.

If you have any questions or suggestions, feel free to contact the author or submit feedback.

---
# MoonNet log

​	

​	

## 更新日志 (Changelog)

### 预告

- 发布MoonNetV1.1版本，当前为预发布模式(预告)，具体时间未知，MoonNet将会陆陆续续新增以下组件：
- 无锁环形缓冲区组件(`ringbuff`),预计MoonNetv1.1即将提供
- 无锁线程池组件(`lfthreadpool`),预计MoonNetv1.1即将提供
- 异步日志组件(`logger`)，并且包含业务程序崩溃日志上传等功能,预计MoonNetv1.2提供
- 内存池(`memorypool`),预计MoonNetv1.3提供
- 支持http/https支持,预计MoonNetv1.3提供
- MoonNetV2可能会选择重构MoonNetV1.0版本的一些组件，比方原始指针替换为智能指针

​	

### v1.0.3

- 因个人习惯以及MoonNetV2版本做准备，至此重命名Threadpool为threadpool

​	

### v1.0.2

- 修正Threadpool中的max_thr_num变量未设置问题

​	

### v1.0.1

- 更新base_event的api，统一多种事件类型的操作
- 重构`server`的事件操作方式，统一多种事件类型操作，**弃用非特殊事件类型操作api(如add_bev(bfevent* bev))，改用addev(base_event* ev)这种通用api**
- 更新copyright

​	

### v1.0.0

- **初始发布**：MoonNet 网络库首次发布，支持基于 Reactor 模型的 TCP 和 UDP 事件处理,封装主从Reactor多线程服务端模块。
- **核心功能**：
  - 事件循环 (`eventloop`) 和事件处理 (`event`) 实现。
  - 多线程支持，通过 `loopthread` 和 `looptpool` 管理多个事件循环。
  - TCP 连接管理，基于 `bfevent` 的缓冲事件处理。
  - UDP 支持，提供 `udpevent` 进行数据接收和发送。
  - 定时器事件处理，通过 `timerevent` 实现定时任务。
  - 信号事件处理，使用 `signalevent` 捕获和处理系统信号。
  - 静态/动态负载均衡
- **工具类**：
  - 缓冲区管理 (`buffer`) 实现高效数据读写。
  - 任务线程池 (`Threadpool`)
  - 平台无关的套接字操作封装 (`wrap`)。
- **服务器框架**：
  - 集成所有组件的 `server` 类，简化服务器搭建流程。


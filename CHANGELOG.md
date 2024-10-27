# MoonNet log

​	

​	

## 更新日志 (Changelog)

### v1.0.0

- **初始发布**：MoonNet 网络库首次发布，支持基于 Reactor 模型的 TCP 和 UDP 事件处理,封装主从Reactor多线程服务端模块。
- **核心功能**：
  - 事件循环 (`eventloop`) 和事件处理 (`event`) 实现。
  - 多线程支持，通过 `loopthread` 和 `looptpool` 管理多个事件循环。
  - TCP 连接管理，基于 `bfevent` 的缓冲事件处理。
  - UDP 支持，提供 `udpevent` 进行数据接收和发送。
  - 定时器事件处理，通过 `timerevent` 实现定时任务。
  - 信号事件处理，使用 `signalevent` 捕获和处理系统信号。
- **工具类**：
  - 缓冲区管理 (`buffer`) 实现高效数据读写。
  - 任务线程池 (`Threadpool`)
  - 平台无关的套接字操作封装 (`wrap`)。
- **服务器框架**：
  - 集成所有组件的 `server` 类，简化服务器搭建流程。


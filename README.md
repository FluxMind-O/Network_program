## 项目介绍:
本项目是一个极简的 echo 服务器，Linux 下的 epoll 机制实现 IO 多路复用，处理多客户端的连接与数据交互，客户端发送的任意数据都会被服务器原样回显


## 功能特性：
- 基于 epoll 的 IO 多路复用，相比 select/poll 性能更强； 
- 集成 POSIX 线程库，支持多线程调度，提升服务响应能力；
- 参考Reactor模式，将核心逻辑与 IO 操作解耦，易于功能扩展 (在未来总工程中支持满血版Reactor模式)；
- 主要是LT，但在回给客户端时使用了ET (小巧思)；

<img width="2094" height="1242" alt="屏幕截图 2026-04-19 194029" src="https://github.com/user-attachments/assets/ffd9a409-6fb4-4de0-831b-62cf46536512" />   
<br>
<img width="2102" height="702" alt="屏幕截图 2026-04-19 194036" src="https://github.com/user-attachments/assets/edd66d12-5b76-45d4-a65a-a9418f6c017a" />


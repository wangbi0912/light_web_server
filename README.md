### 1、基于Linux的轻量级多线程HTTP服务器

**项目地址：** [https://github.com/wangbi0912/light_web_server.git](https://github.com/wangbi0912/light_web_server.git)
**项目描述：** 此项目基于Linux的轻量级Web服务器，应用层实现了一个简单的HTTP服务器，支持静态资源访问。
**项目成果：** 实现了两种网络事件模型，Reactor模式和同步I/O模拟Proactor模式；采用线程池减少线程创建开销问题，实现了一个定时器，定时关闭非活跃连接，降低服务器压力，使用WebBench进行压力测试，在ET和LT模式均实现了C10K并发连接
**静态页面：** [47.102.36.37:10000](47.102.36.37:10000)，未加入前段框架，仅展示用。

ref：游双著：Linux高性能服务器开发

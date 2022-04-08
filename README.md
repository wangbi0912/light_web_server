# light_web_server
An light web server based on multithreading(Thread poll)
The eventLoop model of the server is I/O multiplexing(epoll) + Reactor.
I will update the eventLoop based on epoll + Proactor(simulateb by Synchronize I/O）.
**The socket_fd used the EPOLLONSHOT!**
For personal study.

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include "threadpool.h"
#include "locker.h"
#include <signal.h>
#include "http_conn.h"

#define MAX_FD 65535    //最大文件描述符个数（客户端）
#define MAX_EVENT_NUMBER 10000   //一次监听的最大事件数量

/*addsig函数--信号捕捉
在网络通信时，有一端断开连接，另外一段还继续写数据，
就可能会产生一个信号SIGPIPE，定义一个函数对信号进行处理。

SIGPIPE信号：
管道破裂。这个信号通常在进程间通信产生，
比如采用 FIFO (管道)通信的两个进程，读管道没打开或者意外终止就往管道写，
写进程会收到 SIGPIPE 信号。此外用Socket 通信的两个进程，
写进程在写 Socket 的时候，读进程已经终止。
*/
void addsig(int sig, void(handler)(int)) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(sig, &sa, NULL);
}

//添加监听文件描述符fd到epoll对象epollfd中，http_conn.cpp中实现
extern void addfd(int epollfd, int fd, bool one_shot);

//从epoll对象中删除文件描述符，http_conn.cpp中实现
extern int removefd(int epollfd, int fd);

//修改文件描述符
extern void modifyfd(int epollfd, int fd, int ev);


int main(int argc, char* argv[]) {

    if(argc <= 1) {
        printf("按照以下格式运行: %s port_number\n", basename(argv[0]));
        exit(-1);
    }

    //获取端口号
    int port = atoi(argv[1]);

    //对SIGPIPE信号进行处理，默认忽略该信号
    addsig(SIGPIPE, SIG_IGN);


    //初始化线程池
    threadpool<http_conn>* pool = NULL;
    try {
        pool = new threadpool<http_conn>;
    } catch(...) {
        exit(-1);
    }


    //创建一个数组保存所有客户端的信息
    http_conn* users = new http_conn[MAX_FD];

    //创建监听的socket，并将socket设置为端口复用
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    //绑定ip和端口
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    int bind_ret = bind(listenfd, (struct sockaddr* )&address, sizeof(address));
    if(bind_ret != 0) {
        throw std::exception();
    }

    //监听 listen
    listen(listenfd, 5);

    //创建epoll对象，事件数组，添加
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);

    //将监听的文件描述符添加到epoll对象中
    addfd(epollfd, listenfd, false);
    http_conn::m_epollfd = epollfd;

    while(true) {
        int num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(num < 0 && errno != EINTR) {
            printf("Epoll Failure\n");
            break;
        }

        //循环遍历epoll返回的事件链表
        for(int i = 0; i < num; i++) {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd) {
                struct sockaddr_in client_address;
                socklen_t client_addrlen = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr* )&client_address, &client_addrlen);
                
                if(http_conn::m_user_count >= MAX_EVENT_NUMBER) {
                    //连接数量大于服务器
                    close(listenfd);
                    
                }
                users[connfd].init(connfd, client_address);
            }
            else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                //对方异常断开或者错误
                users[sockfd].close_conn();
            }
            else if(events[i].events & EPOLLIN) {
                if(users[sockfd].read()) {
                    //一次性读取数据
                    pool->append(users + sockfd);
                }
                else {
                    users[sockfd].close_conn();
                }
            }
            else if(events[i].events & EPOLLOUT) {
                //一次性写完数据
                if(users[sockfd].write()) {
                    users[sockfd].close_conn();
                }
                
            }

        }

    }

    close(epollfd);
    close(listenfd);
    delete[] users;
    delete pool;

    return 0;
}
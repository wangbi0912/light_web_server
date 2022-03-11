#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <stdarg.h>
#include <errno.h>
#include "locker.h"



class http_conn {
public:
    
    static int m_epollfd;   //所有socket事件注册到同一个epoll
    static int m_user_count;    //用户的数量

    http_conn() {

    }
    ~http_conn() {
        
    }

    void process();     //处理客户端的请求
    void init(int sockfd, const sockaddr_in& addr);    //初始化http_conn对象
    void close_conn();      //关闭连接
    bool read();        //非阻塞读
    bool write();       //非阻塞写
private:
    int m_socket;   //该HTTP对象连接的socket
    sockaddr_in m_address;   //通信的socket的地址

};



#endif
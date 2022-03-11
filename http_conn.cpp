#include "http_conn.h"


int http_conn::m_epollfd = -1;   //所有socket事件注册到同一个epoll
int http_conn::m_user_count = 0;    //用户的数量


void nonblocking(int fd) {
    int old_flag = fcntl(fd, F_GETFL);
    int new_flag = old_flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_flag);
}


//main.cpp addfd 函数实现
void addfd(int epollfd, int fd, bool one_shot) {
    epoll_event event;
    event.data.fd = fd;

    //默认采用水平触发方式，
    //EPOLLRDHUP : 挂起，对于对端链接异常断开，可以直接在底层处理，不需要移交给上层处理（read返回值）
    event.events = EPOLLIN | EPOLLRDHUP;
    //event.events = EPOLLIN | EPOLLET; 边缘触发

    /*
    one_shot : 即使采用边缘触发(ET)模式，一个socket上的某个事件也可能被触发多次，会出现一些问题。
    比如一个线程读取完一个socket上的数据之后进行处理，在数据处理过程中该socket又出现新的数据可读
    （EPOLLIN再次被触发），此外林外一个线程被唤醒来处理该新数据，就会出现两个线程同时操作一个socket的局面。
    若实现一个socket在任意时刻只能被一个线程处理，可以使用epoll的EPOLLONESHOT实现。
    */
    if(one_shot) {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    
    //若采用ET模式，需要一次性读取出数据，设置文件描述符非阻塞
    nonblocking(fd);
}

//main.cpp removefd 函数实现
int removefd(int epollfd, int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

/*
    main.cpp removefd 函数实现
    需要重置socket上的EPOLLONESHOT事件，确保下一次可读时，EPOLLIN能够被触发。
    ev : 需要修改的事件event；
*/
void modifyfd(int epollfd, int fd, int ev) {
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

//初始化连接
void http_conn::init(int sockfd, const sockaddr_in& addr) {
    // m_socket = sockfd;
    // m_address = addr;
    this->m_socket = sockfd;
    this->m_address = addr;

    //设置端口复用
    int reuse = 1;
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    //添加到epoll对象中
    addfd(m_epollfd, m_socket, true);

    //用户数量增加
    m_user_count ++;
}

//关闭连接
void http_conn::close_conn() {
    if(m_socket != -1) {
        removefd(m_epollfd, m_socket);
        m_socket = -1;
        m_user_count--;
    }
}

//非阻塞读
bool http_conn::read() {
    printf("一次性读取数据\n");
    return true;
}

//非阻塞写
bool http_conn::write() {
    printf("一次性写数据\n");
    return true;
}


//由线程池工作线程调用，处理http请求的入口函数
void http_conn::process() {

    //解析http请求
    printf("parse request, create response\n");
    //生成响应

}
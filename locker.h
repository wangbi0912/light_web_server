//线程同步机制封装类，包含互斥锁类(class locker)、条件变量类(class cond)、信号量类(class sem)。

#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <exception>
#include <semaphore.h>

//互斥锁类
class locker {
public:
    //constructor
    locker() {
        if(pthread_mutex_init(&m_mutex, NULL) != 0)
            throw std::exception();
    }
    //destructor
    ~locker() {
        pthread_mutex_destroy(&m_mutex);
    }

    //lock
    bool lock() {
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    //unlock
    bool unlock() {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

private:
    pthread_mutex_t m_mutex;
};


//条件变量类
class cond {
public:
    cond() {
        if(pthread_cond_init(&m_cond, NULL) != 0) {
            throw std::exception();
        }
    }
    ~cond() {
        pthread_cond_destroy(&m_cond);
    }

    //wait
    bool wait(pthread_mutex_t* m_mutex) {//传m_mutex的引用
        return pthread_cond_wait(&m_cond, m_mutex) == 0;
    }

    //包含超时时间的wait
    bool timewait(pthread_mutex_t* m_mutex, struct timespec t) {
        return pthread_cond_timedwait(&m_cond, m_mutex, &t) == 0;
    }

    //唤醒一个线程
    bool signal(pthread_mutex_t* m_mutex) {
        return pthread_cond_signal(&m_cond) == 0;
    }

    //唤醒所有线程
    bool broadcast(pthread_mutex_t* m_mutex) {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    pthread_cond_t m_cond;
};


//信号量类
class sem {
public:
    sem() {
        if(sem_init(&m_sem, 0, 0) != 0) {
            throw std::exception();
        }
    }

    sem(int num) {
        if(sem_init(&m_sem, 0, num) != 0) {
            throw std::exception();
        }
    }

    ~sem() {
        sem_destroy(&m_sem);
    }

    //等待信号量
    bool wait() {
        return sem_wait(&m_sem) == 0;
    }

    //增加信号量
    bool post() {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};


#endif
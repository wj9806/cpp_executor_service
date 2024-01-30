#ifndef CPP_EXECUTOR_SERVICE_THREAD_POOL_H
#define CPP_EXECUTOR_SERVICE_THREAD_POOL_H
#include "task_queue.h"
#include<iostream>
#include<cstring>

using namespace std;

class thread_pool{
public:
    //创建线程池并初始化
    thread_pool(int min, int max);

    //销毁线程池
    ~thread_pool();

    //给线程池添加任务
    void submit(task task);

    //获取线程池中工作的线程的个数
    int busy_num();

    //获取线程池中活着的线程的个数
    int live_num();
private:
    static void* worker(void* arg);

    static void *manager(void *arg);

    static const int NUM = 2;

    void thread_exit();

    task_queue* taskQ;

    pthread_t _manager_id;//管理者线程id
    pthread_t *_thread_ids;//工作线程id数组
    int _min_num;//最小线程数
    int _max_num;//最大线程数
    int _busy_num;//忙碌线程数
    int _live_num;//存活线程数
    int _exit_num;//要销毁的线程数

    pthread_cond_t _not_empty;//任务队列是不是空了
    pthread_mutex_t _mutex_pool; //锁整个线程池

    int _shutdown; //是不是要销毁整个线程池 销毁1 不销毁0
};


#endif //CPP_EXECUTOR_SERVICE_THREAD_POOL_H

#ifndef CPP_EXECUTOR_SERVICE_TASK_QUEUE_H
#define CPP_EXECUTOR_SERVICE_TASK_QUEUE_H
#include<queue>
#include <pthread.h>
#include<thread>
#include<chrono>

using namespace std;
using callback = void (*)(void* arg);

struct task
{
    task()
    {
        function = nullptr;
        arg = nullptr;
    }
    task(callback f, void* arg)
    {
        this->function = f;
        this->arg = arg;
    }
    callback function;
    void* arg;
};

class task_queue{
public:
    task_queue();
    ~task_queue();

    void add_task(task task);
    void add_task(callback f, void* arg);
    task take_task();
    //获取当前任务个数
    inline size_t task_number()
    {
        return m_taskQ.size();
    }
private:
    pthread_mutex_t m_mutex;
    queue<task> m_taskQ;
};


#endif //CPP_EXECUTOR_SERVICE_TASK_QUEUE_H

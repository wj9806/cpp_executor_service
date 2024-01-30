#include "task_queue.h"

task_queue::task_queue() {
    pthread_mutex_init(&m_mutex, NULL);
}

task_queue::~task_queue() {
    pthread_mutex_destroy(&m_mutex);
}

void task_queue::add_task(task task) {
    pthread_mutex_lock(&m_mutex);
    m_taskQ.push(task);
    pthread_mutex_unlock(&m_mutex);
}

void task_queue::add_task(callback f, void *arg) {
    task t(f, arg);
    add_task(t);
}

task task_queue::take_task() {
    task t;
    pthread_mutex_lock(&m_mutex);
    if (!m_taskQ.empty())
    {
        t = m_taskQ.front();
        m_taskQ.pop();
    }
    pthread_mutex_unlock(&m_mutex);
    return t;
}

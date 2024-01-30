#include<cstdio>
#include "thread_pool.h"
#include<pthread.h>
#include <unistd.h>

void my_task(void* arg)
{
    int num = *(int*)arg;
    printf("thread is working, number = %d, tid = %ld\n", num, pthread_self());
    sleep(1);
}

int main()
{
    //创建线程池
    thread_pool pool(3, 10);
    for (int i = 0; i < 100; ++i)
    {
        int * num = new int(i + 100);
        pool.submit(task(my_task, num));
    }

    sleep(30);

    return 0;
}
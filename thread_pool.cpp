#include "thread_pool.h"

thread_pool::thread_pool(int min, int max) {

    do {
        taskQ = new task_queue;
        if (taskQ == nullptr)
        {
            cout << "malloc taskQ failed" << endl;
            break;
        }
        this->_thread_ids = new pthread_t[max];
        if(this->_thread_ids == nullptr)
        {
            cout << "malloc _thread_ids failed" << endl;
            break;
        }
        //将 pool->_thread_ids 指向的内存块的前 sizeof(pthread_t) * max 个字节设置为0
        memset(this->_thread_ids, 0, sizeof(pthread_t) * max);

        this->_min_num = min;
        this->_max_num = max;
        this->_busy_num = 0;
        this->_live_num = min;
        this->_exit_num = 0;

        if (pthread_mutex_init(&this->_mutex_pool, nullptr) != 0 ||
            pthread_cond_init(&this->_not_empty, nullptr) != 0)
        {
            cout <<"mutex or cond init failed"<< endl;
        }

        this->_shutdown = false;

        /**
         * 创建线程四个参数
         * 1.传入指向pthread_t的指针
         * 2.__attr 设置新线程的属性
         * 3.__start_routine 指定新线程将要执行的线程函数的地址
         * 4.__arg 传递给线程函数 __start_routine 的参数
         */
        pthread_create(&this->_manager_id, NULL, manager, this);
        for (int i = 0; i < this->_live_num; ++i)
        {
            pthread_create(&this->_thread_ids[i], NULL, worker, this);
        }
        return;
    } while (true);

    if (_thread_ids) delete[] _thread_ids;
    if (taskQ) delete taskQ;
}

thread_pool::~thread_pool() {
    this->_shutdown = true;
    //阻塞回收管理者线程
    pthread_join(this->_manager_id, NULL);
    //唤醒阻塞的消费者
    for (int i = 0; i < this->_live_num; ++i) {
        pthread_cond_signal(&this->_not_empty);
    }
    //释放堆内存
    if(taskQ)
    {
        delete taskQ;
    }
    if(_thread_ids)
    {
        delete[] _thread_ids;
    }
    pthread_mutex_destroy(&this->_mutex_pool);
    pthread_cond_destroy(&this->_not_empty);
}

void thread_pool::submit(task task) {
    if(this->_shutdown)
    {
        return;
    }
    //添加任务
    this->taskQ->add_task(task);
    //唤醒消费者
    pthread_cond_signal(&this->_not_empty);
}

int thread_pool::busy_num() {
    pthread_mutex_lock(&this->_mutex_pool);
    int busy =  this->_busy_num;
    pthread_mutex_unlock(&this->_mutex_pool);
    return busy;
}

int thread_pool::live_num() {
    pthread_mutex_lock(&this->_mutex_pool);
    int live = this->_live_num;
    pthread_mutex_unlock(&this->_mutex_pool);
    return live;
}

void *thread_pool::worker(void *arg) {
    thread_pool* pool = static_cast<thread_pool*>(arg);

    while (true)
    {
        pthread_mutex_lock(&pool->_mutex_pool);
        while (pool->taskQ->task_number() == 0 && !pool->_shutdown)
        {
            //阻塞工作线程
            pthread_cond_wait(&pool->_not_empty, &pool->_mutex_pool);
            //判断是不是要销毁线程
            if(pool->_exit_num > 0)
            {
                pool->_exit_num--;
                if(pool->_live_num > pool->_min_num)
                {
                    pool->_live_num--;
                    pthread_mutex_unlock(&pool->_mutex_pool);
                    //线程自杀
                    pool->thread_exit();
                }
            }
        }
        //线程池是否关闭了
        if (pool->_shutdown)
        {
            pthread_mutex_unlock(&pool->_mutex_pool);
            pool->thread_exit();
        }
        //从队列中取出一个任务
        task task = pool->taskQ->take_task();
        pool->_busy_num++;
        //唤醒生产者
        pthread_mutex_unlock(&pool->_mutex_pool);

        cout << "thread " << to_string(pthread_self()) <<  "start working..." << endl;
        //两种都行
        //_task.function(_task.arg);
        task.function(task.arg);
        delete task.arg;
        task.arg = nullptr;
        pthread_mutex_unlock(&pool->_mutex_pool);
        pool->_busy_num--;
        pthread_mutex_unlock(&pool->_mutex_pool);
    }
}

void *thread_pool::manager(void *arg) {
    thread_pool* pool = static_cast<thread_pool*>(arg);
    while (!pool->_shutdown)
    {
        chrono::seconds ch(1);
        this_thread::sleep_for(ch);
        //取出线程池中任务的数量和当前线程的数量
        pthread_mutex_lock(&pool->_mutex_pool);
        int queue_size = pool->taskQ->task_number();
        int live_num = pool->_live_num;
        int busy_num=pool->_busy_num;
        pthread_mutex_unlock(&pool->_mutex_pool);

        //添加线程
        //任务的个数>存活的线程个数 && 存活的线程数<最大线程数
        if(queue_size > live_num && live_num < pool->_max_num)
        {
            pthread_mutex_lock(&pool->_mutex_pool);
            int counter = 0;
            for (int i = 0; i < pool->_max_num && counter < NUM && live_num < pool->_max_num; ++i)
            {
                if(pool->_thread_ids[i] == 0)
                {
                    //创建worker
                    pthread_create(&pool->_thread_ids[i], NULL, worker, pool);
                    counter++;
                    pool->_live_num++;
                }

            }
            pthread_mutex_unlock(&pool->_mutex_pool);
        }
        //销毁线程
        //忙的线程*2 < 存活的线程数 && 存活的线程 > 最小线程数
        if(busy_num* 2 < live_num && live_num > pool->_min_num)
        {
            pthread_mutex_lock(&pool->_mutex_pool);
            pool -> _exit_num = NUM;
            pthread_mutex_unlock(&pool->_mutex_pool);
            //让工作的线程自杀
            for (int i = 0; i < NUM; ++i)
            {
                pthread_cond_signal(&pool->_not_empty);
            }
        }
    }
}

void thread_pool::thread_exit() {
    //获取当前线程id
    pthread_t tid = pthread_self();
    for (int i = 0; i < this->_max_num; ++i)
    {
        if(this->_thread_ids[i] == tid)
        {
            this->_thread_ids[i] = 0;
            cout << "thread_exit() called, " <<to_string(tid) << "  exiting.." << endl;
            break;
        }
    }
    pthread_exit(NULL);
}

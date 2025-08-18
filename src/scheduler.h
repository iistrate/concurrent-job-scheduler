#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "task.h"

enum status_t {
    SUCCESS,
    ABORTED,
    ERROR,
};

using Task_p = std::unique_ptr<Task>;
class Scheduler {

    std::vector<std::thread> threadpool;
    std::mutex mtx;
    std::condition_variable cv;
    bool running = true;
    struct cmp {
        bool operator()(const Task_p& a, const Task_p& b);
    };

    std::priority_queue<Task_p, std::vector<Task_p>, cmp> pq;
    void process();
    void cancel(int tid);
    void start(const int N);
    void stop();
    void join();

public:
    status_t add(Task_p t);
    void run();
};


#endif //SCHEDULER_H
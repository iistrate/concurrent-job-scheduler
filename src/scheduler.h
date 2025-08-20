#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_set>

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
    bool running=false;
    struct cmp {
        bool operator()(const Task* a, const Task* b);
    };

    std::priority_queue<Task*, std::vector<Task*>, cmp> pq;
    std::unordered_set<int> cancelled;

public:
    status_t add(Task_p t);
    void run();
    void cancel(int tid);
    void start(const int N);
    void stop();
    void join();    
};


#endif //SCHEDULER_H
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

    bool running = false;

    std::atomic<bool> stopped = false;
    std::atomic <bool> joined = false;
    struct cmp {
        bool operator()(const Task_p& a, const Task_p& b);
    };

    std::priority_queue<Task_p, std::vector<Task_p>, cmp> pq;
    std::unordered_set<int> cancelled;

public:
    ~Scheduler();
    status_t add(Task_p t);
    void run();
    void cancel(int tid);
    void start(const int N);
    void stop();
    void join();    
};


#endif //SCHEDULER_H
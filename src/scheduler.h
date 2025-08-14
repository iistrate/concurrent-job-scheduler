#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include "task.h"

enum status_t {
    SUCCESS,
    ABORTED,
    ERROR,
};

class Scheduler {
    struct cmp {
        bool operator()(const Task& a, const Task& b);
    };

    std::priority_queue<Task, std::vector<Task>, cmp> pq;
    void process();
    void cancel(int tid);

public:
    status_t add();
    void run();
};


#endif //SCHEDULER_H
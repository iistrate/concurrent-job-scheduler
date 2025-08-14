#include <queue>
#include "scheduler.h"
#include "task.h"
class Scheduler {
    struct cmp {
        bool operator()(const Task& a, const Task& b) {
            if(a.priority == b.priority) {
                return a.timestamp > b.timestamp;
            }
            return a.priority < b.priority;
        }
    };
    std::priority_queue<Task, std::vector<Task>, cmp> pq;
    void process() {

    }
    void cancel(int tid) {

    }
    public:
        status_t add() {

        }
        void run() {

        };
};
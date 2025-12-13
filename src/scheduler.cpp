#include "scheduler.h"

std::atomic<int> Task::counter{0};

bool Scheduler::cmp::operator()(const Task_p& a, const Task_p& b) {
    if (a->getPriority() == b->getPriority()) {
        return a->getTimestamp() > b->getTimestamp();
    }
    return a->getPriority() < b->getPriority();
}

void Scheduler::start(const int N=8) {
    {
        std::lock_guard<std::mutex> lk(mtx);
        running = true;
    }
    threadpool.reserve(N);
    for(int i = 0; i < N; ++i) {
        threadpool.emplace_back(&Scheduler::run, this);
    }
}

void Scheduler::stop() {
    if(stopped.exchange(true)) {
        return;
    }    
    {
        std::lock_guard<std::mutex> lk(mtx);
        running = false;
    }
    cv.notify_all();
}

void Scheduler::join() {
    stop();
    if(joined.exchange(true)) {
        return;
    }
    for(auto &t: threadpool) {
        t.join();        
    }
    threadpool.clear();
}

void Scheduler::cancel(int tid) {
    {
        std::lock_guard<std::mutex> lk(mtx);
        cancelled.insert(tid);
        cv.notify_one();
    }
}

status_t Scheduler::add(Task_p t) {
    if(t == nullptr) return ERROR;
    {
        std::lock_guard<std::mutex> lk(mtx);
        pq.push(std::move(t));
        cv.notify_one();
    }
    return SUCCESS;
}

void Scheduler::run() {
    std::unique_lock<std::mutex> lk(mtx);
    while (true) {
        // Wait until:
        // 1. The queue has items
        // 2. OR the scheduler has stopped (so we can drain the remaining items)
        cv.wait(lk, [this]{ return !pq.empty() || !running; });

        // EXIT CONDITION:
        // If stopped AND nothing left in queue, we are done.
        if (!running && pq.empty()) {
            return;
        }

        // If queue is empty (spurious wake), go back to waiting
        if (pq.empty()) continue;

        // Get the task
        // const_cast is needed because priority_queue::top() returns const&
        Task_p t = std::move(const_cast<Task_p&>(pq.top()));
        pq.pop();

        // Check for cancellation
        if (cancelled.count(t->getId())) {
            cancelled.erase(t->getId());
            continue;
        }

        lk.unlock();
        t->execute();
        lk.lock();
    }
}

Scheduler::~Scheduler() {
    stop();
    join();
}
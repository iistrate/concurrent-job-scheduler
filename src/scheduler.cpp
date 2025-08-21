#include "scheduler.h"

std::atomic<int> Task::counter{0};

bool Scheduler::cmp::operator()(const Task* a, const Task* b) {
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
    {
        std::lock_guard<std::mutex> lk(mtx);
        running = false;
    }
    cv.notify_all();
}

void Scheduler::join() {
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
        pq.push(t.release());
    }
    cv.notify_one();
    return SUCCESS;
}

void Scheduler::run() {
    std::unique_lock<std::mutex> lk(mtx);
    while(running) {
        while (pq.empty() && running) {
             cv.wait(lk);
        }
        if (!running) {
            return;
        }
        Task_p t(pq.top());
        pq.pop();
        if(cancelled.count(t->getId())) {
            cancelled.erase(t->getId());
            continue;
        }
        lk.unlock();
        t->execute();
        lk.lock();
    }
}
#include "scheduler.h"

bool Scheduler::cmp::operator()(const Task_p& a, const Task_p& b) {
    if (a->getPriority() == b->getPriority()) {
        return a->getTimestamp() > b->getTimestamp();
    }
    return a->getPriority() < b->getPriority();
}

void Scheduler::start(const int N=8) {
    mtx.lock();
    running = true;
    mtx.unlock();
    threadpool.reserve(N);
    for(int i = 0; i < N; ++i) {
        threadpool.emplace_back(&Scheduler::run, this);
    }
}

void Scheduler::stop() {
    mtx.lock();
    running = false;
    mtx.unlock();
    cv.notify_all();
}

void Scheduler::join() {
    for(auto &t: threadpool) {
        t.join();        
    }
    threadpool.clear();
}

void Scheduler::cancel(int tid) {
    mtx.lock();
    cancelled.insert(tid);
    cv.notify_one();
    mtx.unlock();
}

status_t Scheduler::add(Task_p t) {
    if(t == nullptr) return ERROR;
    mtx.lock();
    pq.push(t);
    cv.notify_one();
    mtx.unlock();
    return SUCCESS;
}

void Scheduler::run() {
    std::unique_lock<std::mutex> lk(mtx, std::defer_lock);
    while(running) {
        lk.lock();
        while (pq.empty() && running) {
             cv.wait(lk);
        }
        if (!running) {
            lk.unlock();
            return;
        }
        auto t = std::move(const_cast<Task_p&>(pq.top()));
        pq.pop();
        lk.unlock();
        if(!cancelled.count(t->getId())) {
            t->execute();
        }
    }
}
#include "scheduler.h"

bool Scheduler::cmp::operator()(const Task_p& a, const Task_p& b) {
    if (a->getPriority() == b->getPriority()) {
        return a->getTimestamp() > b->getTimestamp();
    }
    return a->getPriority() < b->getPriority();
}

void Scheduler::start(const int N) {
    threadpool.reserve(N);
    for(int i = 0; i < N; ++i) {
        threadpool.emplace_back(&Scheduler::run, this);
    }
}

void Scheduler::stop() {
    std::lock_guard<std::mutex> lk(mtx);
    running = false;
    cv.notify_all();
}

void Scheduler::join() {
    for(auto &t: threadpool) {
        t.join();        
    }
    threadpool.clear();
}

void Scheduler::process() {

}

void Scheduler::cancel(int tid) {

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
    while(running) {
        mtx.lock();
        cv.wait(mtx, [&]{ return !pq.empty() || !running; });
        if (!running) {
            mtx.unlock();
            return;
        }        
        auto t = std::move(const_cast<Task_p&>(pq.top()));
        pq.pop();
        mtx.unlock();        
        t->execute();        
    }
}
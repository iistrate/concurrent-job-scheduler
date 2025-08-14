#include "scheduler.h"

bool Scheduler::cmp::operator()(const Task& a, const Task& b) {
    if (a.priority == b.priority) {
        return a.timestamp > b.timestamp;
    }
    return a.priority < b.priority;
}

void Scheduler::process() {

}

void Scheduler::cancel(int tid) {

}

status_t Scheduler::add() {

}

void Scheduler::run() {

}
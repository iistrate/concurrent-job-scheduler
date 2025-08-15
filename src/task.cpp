#include "task.h"

int Task::getPriority() const {
    return this->priority;
}

void Task::setPriority(int p) {
    priority = p;
}

std::chrono::steady_clock::time_point Task::getTimestamp() const {
    return timestamp;
}
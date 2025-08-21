#ifndef TASK_H
#define TASK_H

#include <chrono>
#include <atomic>
class Task {
    static std::atomic<int> counter;
public:
    explicit Task(int p):priority(p), timestamp(std::chrono::steady_clock::now()), tid(generateNextID()){};
    
    virtual ~Task() = default;
    
    int getPriority() const noexcept { return priority; }
    int getId() const noexcept { return tid; }
    void setPriority(int p) noexcept { priority = p; }
    std::chrono::steady_clock::time_point getTimestamp() const noexcept { return timestamp; }
    virtual void execute() = 0;
    static int generateNextID() {
        return ++counter;
    }
protected:
    int priority;
    int tid;
    std::chrono::steady_clock::time_point timestamp;
};

#endif //TASK_H
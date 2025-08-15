#include <chrono>
class Task {
public:
    Task(int p):priority(p), timestamp(std::chrono::steady_clock::now()){};
    
    virtual ~Task() = default;
    
    int Task::getPriority() const;
    void setPriority(int p);

    std::chrono::steady_clock::time_point getTimestamp() const;
protected:
    int priority;
    std::chrono::steady_clock::time_point timestamp;
};
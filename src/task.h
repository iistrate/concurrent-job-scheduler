#include <chrono>
class Task {
public:
    explicit Task(int p):priority(p), timestamp(std::chrono::steady_clock::now()){};
    
    virtual ~Task() = default;
    
    int getPriority() const noexcept { return priority; };
    void setPriority(int p) noexcept { priority = p; };
    std::chrono::steady_clock::time_point getTimestamp() const noexcept { return timestamp; }
    virtual void execute() = 0;
protected:
    int priority;
    std::chrono::steady_clock::time_point timestamp;
};
#include <chrono>
class Task {
public:
    Task(int p):priority(p), timestamp(std::chrono::steady_clock::now());
    
    virtual ~Task() = default;
    
    int getPriority() const {return priority;}
    void setPriority(int p) {priority = p;}    

    std::chrono::steady_clock::time_point getTimestamp() const {return timestamp;}
protected:
    int priority;
    std::chrono::steady_clock::time_point timestamp;
};
# Concurrent Job Scheduler

A multithreaded C++ job scheduler that manages task execution using a priority queue and thread pool architecture.

## Features

- **Thread Pool**: Configurable number of worker threads
- **Priority-Based Scheduling**: Tasks executed by priority, with timestamp-based tie-breaking
- **Task Cancellation**: Cancel pending tasks before execution
- **Thread-Safe**: Uses mutex and condition variables for synchronization
- **Generic Task Interface**: Virtual base class for custom task implementations

## Architecture

- **Scheduler**: Main orchestrator managing thread pool and task queue
- **Task**: Abstract base class for user defined tasks
- **Priority Queue**: Higher priority tasks execute first; older tasks break ties
- **Cancellation System**: Non blocking task cancellation via ID tracking

## Quick Start

```cpp
#include "scheduler.h"

// Create custom task
class MyTask : public Task {
public:
    MyTask(int priority) : Task(priority) {}
    void execute() override {
        // Your task logic here
    }
};

// Usage
Scheduler scheduler;
scheduler.start(4);  // 4 worker threads

auto task = std::make_unique<MyTask>(10);
scheduler.add(std::move(task));

scheduler.stop();
scheduler.join();
```

## Building

```bash
# Compile (C++14 or later required)
g++ -std=c++14 -pthread src/*.cpp -I src/ -o scheduler

# Run with 8 threads (default)
./scheduler

# Run with custom thread count
./scheduler 4
```

## Project Structure

```
src/
├── main.cpp          # Entry point
├── scheduler.cpp     # Scheduler implementation  
├── scheduler.h       # Scheduler header
├── task.h           # Task base class
└── tests/           # Unit tests
```

## Status

🚧 **Work in Progress** - Core functionality implemented, additional features planned.

## Requirements

- C++14 or later
- pthread support
- Standard library threading support
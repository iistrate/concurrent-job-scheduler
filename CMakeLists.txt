cmake_minimum_required(VERSION 3.10)
project(ConcurrentJobScheduler)

set(CMAKE_CXX_STANDARD 17)

add_executable(scheduler 
    src/main.cpp 
    src/scheduler.cpp 
    src/task.cpp
)

target_link_libraries(scheduler pthread)
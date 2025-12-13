cmake_minimum_required(VERSION 3.10)
project(ConcurrentJobScheduler VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
find_package(GTest REQUIRED)

add_executable(scheduler 
    src/main.cpp 
    src/scheduler.cpp 
)

target_link_libraries(scheduler pthread)

add_executable(scheduler_tests
    tests/test_runner.cpp
    tests/test_scheduler.cpp
    src/scheduler.cpp
)

target_link_libraries(scheduler_tests 
    GTest::gtest 
    GTest::gtest_main
    pthread
)
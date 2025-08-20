#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include <atomic>
#include "../src/scheduler.h"
#include <future>

class TestTask : public Task {
private:
    std::function<void()> action_;
    std::atomic<bool> executed_{false};
    
public:
    TestTask(int priority, std::function<void()> action = [](){}) 
        : Task(priority), action_(action) {}
    
    void execute() override {
        executed_ = true;
        if (action_) action_();
    }
    
    bool wasExecuted() const { 
        return executed_; 
    }
};

class SchedulerTest : public ::testing::Test {
protected:
    void SetUp() override {
        scheduler_ = std::make_unique<Scheduler>();
    }
    
    void TearDown() override {
        if (scheduler_) {
            scheduler_->stop();
            scheduler_->join();
        }
    }
    
    std::unique_ptr<Scheduler> scheduler_;
};


TEST_F(SchedulerTest, BasicTaskExecution) {
    std::promise<void> task_done;
    std::future<void> done_future = task_done.get_future();

    auto task = std::make_unique<TestTask>(1, [&task_done]() {
        task_done.set_value();
    });

    scheduler_->start(1);
    scheduler_->add(std::move(task));

    auto status = done_future.wait_for(std::chrono::seconds(2));
    EXPECT_EQ(status, std::future_status::ready);
}
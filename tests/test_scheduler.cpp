#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <future>

#include "../src/scheduler.h"

class TestTask : public Task {
private:
    std::function<void()> action_;
    std::atomic<bool> executed_{false};
    int delay;
    
public:
    TestTask(int priority, std::function<void()> action = [](){}) 
        : Task(priority), action_(action), delay() {}
    
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


TEST_F(SchedulerTest, IsItFasterWithMoreThreads) {
    const int ntask   = 100;
    const int nthreads= 10;
    const int delay_ms= 200;
    const int delay_margin = 2;
    const int timeout = ntask * delay_ms * 2;

    auto make_task = [&](std::atomic<int>& counter, std::promise<void>& done){
        return [&counter, &done, ntask, delay_ms] {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            if (++counter == ntask) {
                // set_value exactly once
                try { done.set_value(); } catch (...) {}
            }
        };
    };

    // --- Multithreaded ---
    std::atomic<int> completed_multi{0};
    std::promise<void> prom_multi;
    auto fut_multi = prom_multi.get_future();

    scheduler_->start(nthreads);

    auto t0_multi = std::chrono::steady_clock::now();
    for (int i = 0; i < ntask; ++i) {
        auto task = std::make_unique<TestTask>(i, make_task(completed_multi, prom_multi));
        ASSERT_EQ(scheduler_->add(std::move(task)), status_t::SUCCESS); // if you return status
    }
    // Measure until *completion*, not enqueue
    ASSERT_EQ(fut_multi.wait_for(std::chrono::seconds(timeout)), std::future_status::ready);
    auto t1_multi = std::chrono::steady_clock::now();
    auto dur_multi = std::chrono::duration_cast<std::chrono::milliseconds>(t1_multi - t0_multi);

    scheduler_->stop();
    scheduler_->join(); // IMPORTANT: clean shutdown before restart

    // --- Single-threaded ---
    std::atomic<int> completed_single{0};
    std::promise<void> prom_single;
    auto fut_single = prom_single.get_future();

    scheduler_->start(1);

    auto t0_single = std::chrono::steady_clock::now();
    for (int i = 0; i < ntask; ++i) {
        auto task = std::make_unique<TestTask>(i, make_task(completed_single, prom_single));
        ASSERT_EQ(scheduler_->add(std::move(task)), status_t::SUCCESS);
    }
    // 100 * 100ms = 10s theoretical; give margin:
    ASSERT_EQ(fut_single.wait_for(std::chrono::seconds(timeout)), std::future_status::ready);
    auto t1_single = std::chrono::steady_clock::now();
    auto dur_single = std::chrono::duration_cast<std::chrono::milliseconds>(t1_single - t0_single);

    scheduler_->stop();
    scheduler_->join();

    std::cout << "Multithread (" << nthreads << ")  : " << dur_multi.count()  << " ms\n";
    std::cout << "Singlethread (1)       : " << dur_single.count() << " ms\n";
    std::cout << "Speedup                 : " << (double)dur_single.count() / dur_multi.count() << "x\n";

    EXPECT_LT(dur_multi.count(), dur_single.count());
}
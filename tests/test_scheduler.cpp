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
    Scheduler scheduler_st;
    std::atomic<int> completed_single{0};
    std::promise<void> prom_single;
    auto fut_single = prom_single.get_future();

    scheduler_st.start(1);

    auto t0_single = std::chrono::steady_clock::now();
    for (int i = 0; i < ntask; ++i) {
        auto task = std::make_unique<TestTask>(i, make_task(completed_single, prom_single));
        ASSERT_EQ(scheduler_st.add(std::move(task)), status_t::SUCCESS);
    }
    // 100 * 100ms = 10s theoretical; give margin:
    ASSERT_EQ(fut_single.wait_for(std::chrono::seconds(timeout)), std::future_status::ready);
    auto t1_single = std::chrono::steady_clock::now();
    auto dur_single = std::chrono::duration_cast<std::chrono::milliseconds>(t1_single - t0_single);

    std::cout << "Multithread (" << nthreads << ")  : " << dur_multi.count()  << " ms\n";
    std::cout << "Singlethread (1)       : " << dur_single.count() << " ms\n";
    std::cout << "Speedup                 : " << (double)dur_single.count() / dur_multi.count() << "x\n";

    EXPECT_LT(dur_multi.count(), dur_single.count());
}


TEST_F(SchedulerTest, PriorityExecutionOrder) {
    // 1. Start with 1 thread so we can deterministicly control execution order
    scheduler_->start(1);

    std::vector<int> execution_order;
    std::mutex vector_mutex;
    std::promise<void> gatekeeper_start, gatekeeper_end;

    // 2. Add a "Gatekeeper" task to block the single thread
    auto gatekeeper = std::make_unique<TestTask>(50, [&]() {
        gatekeeper_start.set_value(); // Signal that we are running
        gatekeeper_end.get_future().wait(); // Block until allowed to finish
    });
    scheduler_->add(std::move(gatekeeper));

    // Wait for gatekeeper to occupy the thread
    gatekeeper_start.get_future().wait();

    // 3. Now that the thread is busy, queue up tasks
    // Add Low Priority first (should run LAST)
    auto low_prio = std::make_unique<TestTask>(10, [&]() {
        std::lock_guard<std::mutex> lock(vector_mutex);
        execution_order.push_back(10);
    });
    
    // Add High Priority second (should run FIRST)
    auto high_prio = std::make_unique<TestTask>(90, [&]() {
        std::lock_guard<std::mutex> lock(vector_mutex);
        execution_order.push_back(90);
    });

    scheduler_->add(std::move(low_prio));
    scheduler_->add(std::move(high_prio));

    // 4. Unblock the gatekeeper
    gatekeeper_end.set_value();

    // 5. Stop ensures everything processes before we verify
    scheduler_->stop();
    scheduler_->join();

    // Verify High Prio (90) ran before Low Prio (10)
    ASSERT_EQ(execution_order.size(), 2);
    EXPECT_EQ(execution_order[0], 90);
    EXPECT_EQ(execution_order[1], 10);
}

TEST_F(SchedulerTest, FifoTieBreaking) {
    // Verify that tasks with SAME priority run in FIFO order
    scheduler_->start(1);

    std::vector<int> execution_order;
    std::mutex vector_mutex;
    std::promise<void> gatekeeper_run, gatekeeper_release;

    // Block the thread
    scheduler_->add(std::make_unique<TestTask>(100, [&]() {
        gatekeeper_run.set_value();
        gatekeeper_release.get_future().wait();
    }));
    gatekeeper_run.get_future().wait();

    // Add 3 tasks with SAME priority (1)
    for(int i = 1; i <= 3; ++i) {
        scheduler_->add(std::make_unique<TestTask>(1, [&, i]() {
            std::lock_guard<std::mutex> lock(vector_mutex);
            execution_order.push_back(i);
        }));
        // Small sleep to ensure timestamps are distinct
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); 
    }

    gatekeeper_release.set_value(); // Unleash the queue
    scheduler_->stop();
    scheduler_->join();

    // Expect: 1, 2, 3
    std::vector<int> expected = {1, 2, 3};
    EXPECT_EQ(execution_order, expected);
}

TEST_F(SchedulerTest, TaskCancellation) {
    scheduler_->start(1);
    
    std::promise<void> gatekeeper_ready, gatekeeper_finish;
    
    // 1. Block the thread so the next task sits in the queue
    scheduler_->add(std::make_unique<TestTask>(10, [&]() {
        gatekeeper_ready.set_value();
        gatekeeper_finish.get_future().wait();
    }));
    gatekeeper_ready.get_future().wait();

    // 2. Create and queue the victim task
    auto task_ptr = std::make_unique<TestTask>(5); 
    int task_id = task_ptr->getId(); // Capture ID before move
    
    // We pass a raw pointer purely for verification (unsafe in prod, okay for this test scope)
    TestTask* raw_ptr = task_ptr.get(); 
    
    scheduler_->add(std::move(task_ptr));

    // 3. Cancel it immediately
    scheduler_->cancel(task_id);

    // 4. Release the thread
    gatekeeper_finish.set_value();
    
    scheduler_->stop();
    scheduler_->join();

    // 5. Verify the task was NOT executed
    EXPECT_FALSE(raw_ptr->wasExecuted());
}
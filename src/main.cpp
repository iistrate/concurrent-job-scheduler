#include <iostream>
#include "scheduler.h"

int main(int argc, char* argv[]) {
    std::cout << "Job scheduler starting..." << std::endl;

    const int N = (argc > 1) ? std::stoi(argv[1]) : 8;
    
    Scheduler s;
    s.start(N);

    std::vector<std::unique_ptr<Task>> tasks;
    
    for(auto& task: tasks) {
        s.add(std::move(task));
    }

    s.stop();
    s.join();
    return 0;
}


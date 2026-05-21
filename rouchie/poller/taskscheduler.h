#pragma once

#include <memory>
#include <functional>

#include "base/sem.h"

using Task = std::function<void()>;

class TaskScheduler {
public:
    using Ptr = std::shared_ptr<TaskScheduler>;
    
    TaskScheduler() = default;
    virtual ~TaskScheduler() = default;

    virtual void Async(const Task& task) = 0;
    virtual void FirstAsync(const Task& task) = 0;

    void Sync(const Task &task);
    void FirstSync(const Task& task);
};

inline void TaskScheduler::Sync(const Task &task) {
    Semaphore sem;
    Async([&]() {
        task();
        sem.Post();
    });
    sem.Wait();
}

inline void TaskScheduler::FirstSync(const Task &task) {
    Semaphore sem;
    FirstAsync([&]() {
        task();
        sem.Post();
    });
    sem.Wait();
}

#include <iostream>

#include "task.h"
#include "uv_scheduler.h"

#include <spdlog/spdlog.h>

#include "sem.h"

void TaskExecutorInterface::Sync(const Task& task) {
    Semaphore sem;
    Execute([&]() {
        task();
        sem.Post();
    });
    sem.Wait();
}

void TaskExecutorInterface::FirstSync(const Task& task) {
    Semaphore sem;
    FirstExecute([&]() {
        task();
        sem.Post();
    });
    sem.Wait();
}

std::shared_ptr<TaskExecutor> TaskExecutorImp::Create() {
    std::shared_ptr<TaskExecutorImp> executor(new TaskExecutorImp());
    executor->Start();
    return executor;
}

TaskExecutorImp::TaskExecutorImp() {
    // 使用策略模式，创建具体的调度器实现
    _scheduler = UvScheduler::Create();
    SPDLOG_INFO("TaskExecutorImp");
}

TaskExecutorImp::~TaskExecutorImp() {
    const std::thread::id id = std::this_thread::get_id();
    if (id == _loopThreadID) {
        _scheduler->Stop();
        _loopThread->detach();
    } else {
        Semaphore sem;
        _scheduler->PostHighPriority([this, &sem]() {
            _scheduler->Stop();
            sem.Post();
        });
        sem.Wait();
        _loopThread->join();
    }
    SPDLOG_INFO("~TaskExecutorImp");
}

void TaskExecutorImp::Execute(const Task &task) {
    _scheduler->Post(task);
}

void TaskExecutorImp::FirstExecute(const Task &task) {
    _scheduler->PostHighPriority(task);
}

void TaskExecutorImp::Start() {
    Semaphore sem;
    auto scheduler = _scheduler;
    _loopThread = std::make_shared<std::thread>([&, scheduler]() {
        _loopThreadID = std::this_thread::get_id();
        sem.Post();
        _scheduler->Start();
    });
    sem.Wait();
}

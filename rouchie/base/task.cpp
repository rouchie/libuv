#include <iostream>

#include "task.h"

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
    _event = Event::Create();
    SPDLOG_INFO("TaskExecutorImp");
}

TaskExecutorImp::~TaskExecutorImp() {
    const std::thread::id id = std::this_thread::get_id();
    if (id == _loopThreadID) {
        _event->Stop();
        _loopThread->detach();
    } else {
        Semaphore sem;
        _event->FirstExecute([this, &sem]() {
            _event->Stop();
            sem.Post();
        });
        sem.Wait();
        _loopThread->join();
    }
    SPDLOG_INFO("~TaskExecutorImp");
}

void TaskExecutorImp::Execute(const Task &task) {
    _event->Execute(task);
}

void TaskExecutorImp::FirstExecute(const Task &task) {
    _event->FirstExecute(task);
}

void TaskExecutorImp::Start() {
    Semaphore sem;
    auto event = _event;
    _loopThread = std::make_shared<std::thread>([&, event]() {
        _loopThreadID = std::this_thread::get_id();
        sem.Post();
        _event->Start();
    });
    sem.Wait();
}

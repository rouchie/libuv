#include <iostream>
#include <utility>

#include "task.h"

#include <ppltasks.h>

#include "uvscheduler.h"

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
    const auto scheduler = UvScheduler::Create();
    std::shared_ptr<TaskExecutorImp> executor(new TaskExecutorImp(scheduler));
    executor->Start();
    return executor;
}

TaskExecutorImp::TaskExecutorImp(TaskScheduler::Ptr scheduler) {
    // 使用策略模式，创建具体的调度器实现
    _scheduler = std::move(scheduler);
    SPDLOG_TRACE("TaskExecutorImp");
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
    SPDLOG_TRACE("~TaskExecutorImp");
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

std::shared_ptr<EventPoller> EventPoller::Create() {
    const auto scheduler = UvScheduler::Create();
    std::shared_ptr<EventPoller> executor(new EventPoller(scheduler, scheduler));
    executor->Start();
    return executor;
}

NET::Ptr EventPoller::TcpStart(int port, const std::string &host, int backlog) const {
    return _netScheduler->TcpStart(host, port, backlog);
}

EventPoller::EventPoller(TaskScheduler::Ptr taskScheduler, NetScheduler::Ptr netScheduler) : TaskExecutorImp(std::move(taskScheduler)), _netScheduler(std::move(netScheduler)) {
}


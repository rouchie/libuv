#include "task.h"
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
}

TaskExecutorImp::~TaskExecutorImp() = default;

void TaskExecutorImp::Execute(const Task &task) {
    _event->Execute(task);
}

void TaskExecutorImp::FirstExecute(const Task &task) {
    _event->FirstExecute(task);
}

void TaskExecutorImp::Start() {
    Semaphore sem;
    _mainThread = std::make_shared<std::thread>([&]() {
        sem.Post();
        _event->Start();
    });
    sem.Wait();
}

#pragma once

#include <thread>
#include <memory>

#include "task_scheduler.h"

class TaskExecutorInterface {
public:
    TaskExecutorInterface() = default;
    virtual ~TaskExecutorInterface() = default;

    virtual void Execute(const Task& task) = 0;
    virtual void FirstExecute(const Task& task) = 0;

    void Sync(const Task &task);
    void FirstSync(const Task& task);
};

// 中间接口，可以添加其他功能，比如负载统计
class TaskExecutor : public TaskExecutorInterface {
public:
    TaskExecutor() = default;
    ~TaskExecutor() override = default;
};

class TaskExecutorImp : public TaskExecutor, public std::enable_shared_from_this<TaskExecutorImp> {
public:
    static std::shared_ptr<TaskExecutor> Create();
    ~TaskExecutorImp() override;

protected:
    TaskExecutorImp();
    void Start();

public:
    void Execute(const Task& task) override;
    void FirstExecute(const Task& task) override;

private:
    std::shared_ptr<std::thread> _loopThread;
    std::thread::id _loopThreadID;

    // 使用策略模式，依赖抽象而非具体实现
    TaskScheduler::Ptr _scheduler;
};

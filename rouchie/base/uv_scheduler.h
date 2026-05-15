#pragma once

#include <memory>
#include <functional>
#include <mutex>
#include <list>

#include "uv.h"
#include "task_scheduler.h"

/**
 * 基于 libuv 的任务调度器实现
 */
class UvScheduler : public TaskScheduler {
public:
    static std::shared_ptr<UvScheduler> Create();
    ~UvScheduler() override;

    void Start() override;
    void Stop() override;
    void Post(Task task) override;
    void PostHighPriority(Task task) override;

private:
    UvScheduler();
    void Init();
    void HandleTasks();

private:
    std::shared_ptr<uv_loop_t> _loop;
    uv_async_t _async;

    std::mutex _mutex;
    std::list<Task> _tasks;
};

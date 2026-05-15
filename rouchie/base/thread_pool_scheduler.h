#pragma once

#include <memory>
#include <functional>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "task_scheduler.h"

/**
 * 基于线程池的任务调度器实现（示例）
 * 展示如何扩展不同的调度策略
 */
class ThreadPoolScheduler : public TaskScheduler {
public:
    static std::shared_ptr<ThreadPoolScheduler> Create(size_t threadCount = 4);
    ~ThreadPoolScheduler() override;

    void Start() override;
    void Stop() override;
    void Post(Task task) override;
    void PostHighPriority(Task task) override;

private:
    ThreadPoolScheduler(size_t threadCount);
    void WorkerFunction();

private:
    size_t _threadCount;
    std::vector<std::thread> _workers;
    
    std::priority_queue<Task, std::vector<Task>, std::function<bool(const Task&, const Task&)>> _highPriorityQueue;
    std::queue<Task> _normalQueue;
    
    std::mutex _mutex;
    std::condition_variable _cv;
    bool _stop;
};

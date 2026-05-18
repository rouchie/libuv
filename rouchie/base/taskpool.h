#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>
#include <algorithm>

#include "task.h"

/**
 * 任务池类
 * 管理一组 TaskExecutor 实例，实现负载均衡和资源控制
 */
class TaskExecutorPool : public std::enable_shared_from_this<TaskExecutorPool> {
public:
    static TaskExecutorPool& Instance();
    static void SetPoolSize(unsigned int size);

    ~TaskExecutorPool() =  default;
    TaskExecutor::Ptr GetExecutor();

private:
    TaskExecutorPool();

private:
    std::vector<std::shared_ptr<TaskExecutor>> _executors;  // executor 池
    std::atomic<size_t> _nextIndex{0};  // 轮询索引
};

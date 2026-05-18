#include "taskpool.h"
#include "sem.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

#include "task.h"

static unsigned int g_taskPoolSize = 0;

/**
 * 单例
 * @return 任务池
 */
TaskExecutorPool &TaskExecutorPool::Instance() {
    static TaskExecutorPool pool;
    return pool;
}

/**
 * 任务池大小
 * @param size 默认cpu核数，内部限制 1/2 cpu核数 ~ 2 * cpu核数
 */
void TaskExecutorPool::SetPoolSize(unsigned int size) {
    const auto cpus = std::thread::hardware_concurrency();

    if (size == 0) {
        g_taskPoolSize = cpus;
    } else if (cpus * 2 < size) {
        g_taskPoolSize = cpus * 2;
    } else if (size < cpus / 2) {
        g_taskPoolSize = cpus / 2;
    } else {
        g_taskPoolSize = size;
    }
}

TaskExecutorPool::TaskExecutorPool() {
    if (g_taskPoolSize == 0) {
        g_taskPoolSize = std::thread::hardware_concurrency();
    }
    for (unsigned int i = 0; i < g_taskPoolSize; ++i) {
        _executors.emplace_back(EventPoller::Create());
    }
}

std::shared_ptr<TaskExecutor> TaskExecutorPool::GetExecutor() {
    // 轮询策略：选择下一个 executor
    const size_t index = _nextIndex.fetch_add(1) % _executors.size();
    return _executors[index];
}

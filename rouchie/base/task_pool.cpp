#include "task_pool.h"
#include "sem.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

TaskPool & TaskPool::Instance() {
    static TaskPool pool;
    return pool;
}

std::shared_ptr<TaskPool> TaskPool::Create(size_t minSize, size_t maxSize) {
    if (minSize == 0) {
        throw std::invalid_argument("minSize must be greater than 0");
    }
    if (maxSize < minSize) {
        throw std::invalid_argument("maxSize must be greater than or equal to minSize");
    }
    
    std::shared_ptr<TaskPool> pool(new TaskPool(minSize, maxSize));
    pool->InitializePool();
    pool->Start();
    return pool;
}

TaskPool::TaskPool() {
    SPDLOG_INFO("TaskPool created with minSize={}, maxSize={}", minSize, maxSize);
}

TaskPool::TaskPool(size_t minSize, size_t maxSize)
    : _minSize(minSize)
    , _maxSize(maxSize) {
    SPDLOG_INFO("TaskPool created with minSize={}, maxSize={}", minSize, maxSize);
}

TaskPool::~TaskPool() {
    Stop();
    SPDLOG_INFO("~TaskPool");
}

void TaskPool::Start() {
    if (_started.exchange(true)) {
        SPDLOG_WARN("TaskPool already started");
        return;
    }
    
    _stopped.store(false);
    SPDLOG_INFO("TaskPool started with {} executors", _executors.size());
}

void TaskPool::Stop() {
    if (!_started.load() || _stopped.exchange(true)) {
        return;
    }
    
    SPDLOG_INFO("TaskPool stopping...");
    
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto& executor : _executors) {
        if (executor) {
            executor.reset();
        }
    }
    _executors.clear();
    
    _started.store(false);
    SPDLOG_INFO("TaskPool stopped");
}

void TaskPool::Execute(const Task& task) {
    if (_stopped.load()) {
        SPDLOG_WARN("TaskPool is stopped, cannot execute task");
        return;
    }
    
    auto executor = SelectExecutor();
    if (executor) {
        _totalSubmittedTasks.fetch_add(1);
        executor->Execute(task);
    }
}

void TaskPool::FirstExecute(const Task& task) {
    if (_stopped.load()) {
        SPDLOG_WARN("TaskPool is stopped, cannot execute high priority task");
        return;
    }
    
    auto executor = SelectExecutor();
    if (executor) {
        _totalSubmittedTasks.fetch_add(1);
        executor->FirstExecute(task);
    }
}

void TaskPool::Sync(const Task& task) {
    if (_stopped.load()) {
        SPDLOG_WARN("TaskPool is stopped, cannot sync task");
        return;
    }
    
    Semaphore sem;
    Execute([&]() {
        task();
        sem.Post();
    });
    sem.Wait();
}

size_t TaskPool::GetActiveSize() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _executors.size();
}

std::shared_ptr<TaskExecutor> TaskPool::SelectExecutor() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (_executors.empty()) {
        SPDLOG_ERROR("No executors available in pool");
        return nullptr;
    }
    
    // 轮询策略：选择下一个 executor
    size_t index = _nextIndex.fetch_add(1) % _executors.size();
    return _executors[index];
}

void TaskPool::InitializePool() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    SPDLOG_INFO("Initializing task pool with {} executors", _minSize);
    
    for (size_t i = 0; i < _minSize; ++i) {
        auto executor = TaskExecutorImp::Create();
        _executors.push_back(executor);
    }
    
    SPDLOG_INFO("Task pool initialized with {} executors", _executors.size());
}

void TaskPool::ExpandPool() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (_executors.size() >= _maxSize) {
        SPDLOG_WARN("Cannot expand pool: reached max size ({})", _maxSize);
        return;
    }
    
    size_t newSize = std::min(_executors.size() + 1, _maxSize);
    SPDLOG_INFO("Expanding task pool from {} to {} executors", _executors.size(), newSize);
    
    while (_executors.size() < newSize) {
        auto executor = TaskExecutorImp::Create();
        _executors.push_back(executor);
    }
}

void TaskPool::ShrinkPool() {
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (_executors.size() <= _minSize) {
        SPDLOG_WARN("Cannot shrink pool: reached min size ({})", _minSize);
        return;
    }
    
    size_t newSize = std::max(_executors.size() - 1, _minSize);
    SPDLOG_INFO("Shrinking task pool from {} to {} executors", _executors.size(), newSize);
    
    while (_executors.size() > newSize) {
        _executors.pop_back();
    }
}

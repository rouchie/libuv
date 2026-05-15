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
class TaskPool : public std::enable_shared_from_this<TaskPool> {
public:
    using Ptr = std::shared_ptr<TaskPool>;

    static TaskPool& Instance();
    
    /**
     * 创建任务池
     * @param minSize 最小 executor 数量
     * @param maxSize 最大 executor 数量
     * @return TaskPool 实例
     */
    static Ptr Create(size_t minSize = 2, size_t maxSize = 8);
    
    ~TaskPool();
    
    /**
     * 启动任务池
     */
    void Start();
    
    /**
     * 停止任务池
     */
    void Stop();
    
    /**
     * 提交任务到任务池（自动选择 executor）
     * @param task 要执行的任务
     */
    void Execute(const Task& task);
    
    /**
     * 提交高优先级任务到任务池
     * @param task 要执行的任务
     */
    void FirstExecute(const Task& task);
    
    /**
     * 同步执行任务
     * @param task 要执行的任务
     */
    void Sync(const Task& task);
    
    /**
     * 获取当前活跃的 executor 数量
     * @return executor 数量
     */
    size_t GetActiveSize() const;
    
    /**
     * 获取任务池配置的最小大小
     * @return 最小大小
     */
    size_t GetMinSize() const { return _minSize; }
    
    /**
     * 获取任务池配置的最大大小
     * @return 最大大小
     */
    size_t GetMaxSize() const { return _maxSize; }
    
    /**
     * 获取已提交的任务总数
     * @return 任务总数
     */
    uint64_t GetTotalSubmittedTasks() const { return _totalSubmittedTasks.load(); }

private:
    TaskPool();
    TaskPool(size_t minSize, size_t maxSize);

    /**
     * 选择下一个 executor（轮询策略）
     * @return 选中的 executor
     */
    std::shared_ptr<TaskExecutor> SelectExecutor();
    
    /**
     * 初始化 executor 池
     */
    void InitializePool();
    
    /**
     * 扩展任务池（增加 executor）
     */
    void ExpandPool();
    
    /**
     * 收缩任务池（减少 executor）
     */
    void ShrinkPool();

private:
    size_t _minSize;                    // 最小 executor 数量
    size_t _maxSize;                    // 最大 executor 数量
    std::vector<std::shared_ptr<TaskExecutor>> _executors;  // executor 池
    mutable std::mutex _mutex;          // 保护 executor 池的互斥锁
    
    std::atomic<size_t> _nextIndex{0};  // 轮询索引
    std::atomic<uint64_t> _totalSubmittedTasks{0};  // 已提交任务总数
    
    std::atomic<bool> _started{false};  // 是否已启动
    std::atomic<bool> _stopped{false};  // 是否已停止

    static int 
};

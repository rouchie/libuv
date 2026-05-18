#pragma once

#include <memory>
#include <functional>

using Task = std::function<void()>;

/**
 * 任务调度器策略接口
 * 支持不同的任务调度实现（libuv、线程池等）
 */
class TaskScheduler {
public:
    using Ptr = std::shared_ptr<TaskScheduler>;
    
    virtual ~TaskScheduler() = default;
    
    /**
     * 启动事件循环/调度器
     */
    virtual void Start() = 0;
    
    /**
     * 停止事件循环/调度器
     */
    virtual void Stop() = 0;
    
    /**
     * 提交普通优先级任务
     */
    virtual void Post(Task task) = 0;
    
    /**
     * 提交高优先级任务（优先执行）
     */
    virtual void PostHighPriority(Task task) = 0;

protected:
    TaskScheduler() = default;
};

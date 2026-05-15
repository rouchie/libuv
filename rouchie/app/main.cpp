#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include "uv.h"
#include "base/task.h"
#include "base/sem.h"
#include "base/task_scheduler.h"
#include "base/uv_scheduler.h"
#include "base/task_pool.h"

#include "spdlog/spdlog.h"

int main() {
    SPDLOG_INFO("=== TaskPool Example ===");
    
    // 创建任务池，最小2个executor，最大8个executor
    auto taskPool = TaskPool::Create(2, 8);
    
    SPDLOG_INFO("TaskPool created with active size: {}", taskPool->GetActiveSize());
    
    // 提交多个任务到任务池
    for (int i = 0; i < 20; ++i) {
        taskPool->Execute([i]() {
            const auto id = std::this_thread::get_id();
            std::stringstream oss;
            oss << id;
            SPDLOG_INFO("Task {} executed on thread: {}", i, oss.str());
            uv_sleep(100); // 模拟任务执行时间
        });
        
        // 每隔5个任务提交一个高优先级任务
        if (i % 5 == 0) {
            taskPool->FirstExecute([i]() {
                SPDLOG_INFO("High priority task {} executed", i);
            });
        }
    }
    
    SPDLOG_INFO("Submitted {} tasks to pool", taskPool->GetTotalSubmittedTasks());
    
    // 等待一段时间让任务执行
    uv_sleep(3000);
    
    SPDLOG_INFO("TaskPool stats - Active executors: {}, Total submitted: {}", 
                taskPool->GetActiveSize(), 
                taskPool->GetTotalSubmittedTasks());
    
    // 同步执行示例
    SPDLOG_INFO("Executing sync task...");
    taskPool->Sync([]() {
        SPDLOG_INFO("Sync task executed");
        uv_sleep(500);
    });
    SPDLOG_INFO("Sync task completed");
    
    // 停止任务池
    taskPool->Stop();
    
    SPDLOG_INFO("done");

    return 0;
}

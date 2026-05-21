#pragma once

#include <memory>
#include <mutex>
#include <list>

#include "taskscheduler.h"
#include "loopcontext.h"
#include "basecontext.h"

class AsyncContext {
public:
    using Ptr = std::shared_ptr<AsyncContext>;

    explicit AsyncContext(const LoopContext::Ptr &loop);
    ~AsyncContext();

    void Async(const Task &task);
    void FirstAsync(const Task &task);

    void AsyncCallback();

private:
    BaseContext* _ctx = nullptr;
    LoopContext::Ptr _loop;

    std::mutex _mutex;
    std::list<Task> _tasks;
};
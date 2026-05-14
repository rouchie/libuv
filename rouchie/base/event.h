#pragma once

#include <memory>
#include <functional>
#include <mutex>
#include <list>

#include "uv.h"

using Task = std::function<void()>;

class Event {
public:
    static std::shared_ptr<Event> Create();
    ~Event();

    void Start();
    void Stop();

    void Execute(const Task& task);
    void FirstExecute(const Task& task);

private:
    Event();

    void Init();
    void HandleTasks();

private:
    std::shared_ptr<uv_loop_t> _loop;
    uv_async_t _async;

    std::mutex _mutex;
    std::list<Task> _tasks;
};

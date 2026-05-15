#include "uv_scheduler.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <spdlog/spdlog.h>

std::shared_ptr<UvScheduler> UvScheduler::Create() {
    std::shared_ptr<UvScheduler> scheduler(new UvScheduler());
    scheduler->Init();
    return scheduler;
}

UvScheduler::~UvScheduler() {
    SPDLOG_INFO("~UvScheduler");
}

void UvScheduler::Stop() {
    if (_loop) {
        uv_close(reinterpret_cast<uv_handle_t *>(&_async), nullptr);
        uv_stop(_loop.get());
    }
}

void UvScheduler::Start() {
    std::stringstream oss;
    oss << std::this_thread::get_id();

    SPDLOG_INFO("UvScheduler start with {} thread", oss.str());

    uv_run(_loop.get(), UV_RUN_DEFAULT);
    _loop.reset();

    SPDLOG_INFO("UvScheduler {} thread done", oss.str());
}

void UvScheduler::Post(Task task) {
    std::lock_guard<std::mutex> lock(_mutex);
    _tasks.push_back(task);
    uv_async_send(&_async);
}

void UvScheduler::PostHighPriority(Task task) {
    std::lock_guard<std::mutex> lock(_mutex);
    _tasks.push_front(task);
    uv_async_send(&_async);
}

void UvScheduler::HandleTasks() {
    std::list<Task> tasks;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        tasks.swap(_tasks);
    }
    for (const auto &task: tasks) {
        task();
    }
}

UvScheduler::UvScheduler() : _loop(nullptr), _async() {
    _loop = std::shared_ptr<uv_loop_t>(new uv_loop_t(), [](uv_loop_t *loop) {
        uv_loop_close(loop);
        delete loop;
    });

    const int ret = uv_loop_init(_loop.get());
    if (ret != 0) {
        throw std::runtime_error("Failed to init uv loop: " + std::string(uv_strerror(ret)));
    }

    SPDLOG_INFO("UvScheduler");
}

void UvScheduler::Init() {
    _async.data = this;
    const int ret = uv_async_init(_loop.get(), &_async, [](uv_async_t *async) {
        const auto scheduler = static_cast<UvScheduler *>(async->data);
        scheduler->HandleTasks();
    });
    
    if (ret != 0) {
        throw std::runtime_error("Failed to init async: " + std::string(uv_strerror(ret)));
    }
}

#include "loopcontext.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "base/sem.h"

LoopContext::LoopContext() {
    uv_loop_init(&_loop);
    SPDLOG_INFO("LoopContext: {}", Name());
}

LoopContext::~LoopContext() {
    uv_loop_close(&_loop);
    SPDLOG_INFO("~LoopContext: {}", Name());
}

uv_loop_t *LoopContext::Handle() {
    return &_loop;
}

bool LoopContext::IsLoopThread() const {
    return _loopThreadID == std::this_thread::get_id();
}

std::string LoopContext::Name() const {
    return fmt::format("{}", fmt::ptr(&_loop));
}

void LoopContext::Start() {
    Semaphore sem;
    auto self = shared_from_this();
    _loopThread = std::make_shared<std::thread>([&, self]() {
        _loopThreadID = std::this_thread::get_id();
        sem.Post();
        uv_run(&_loop, UV_RUN_DEFAULT);
        SPDLOG_INFO("Thread Done: {}", self->Name());
    });
    _loopThread->detach();
    sem.Wait();
}

void LoopContext::Stop() {
    uv_stop(&_loop);
}


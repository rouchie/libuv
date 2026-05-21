#pragma once

#include <string>
#include <thread>

#include "uv.h"

class LoopContext : public std::enable_shared_from_this<LoopContext> {
public:
    using Ptr = std::shared_ptr<LoopContext>;

    LoopContext();
    ~LoopContext();

    void Start();
    void Stop();

    uv_loop_t *Handle();
    bool IsLoopThread() const;
    std::string Name() const;

private:
    uv_loop_t _loop{};
    std::shared_ptr<std::thread> _loopThread;
    std::thread::id _loopThreadID;
};
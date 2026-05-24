#pragma once

#include <functional>
#include <memory>

using TimerTask = std::function<bool()>;

class Timer {
public:
    using Ptr = std::shared_ptr<Timer>;
    Timer() = default;
    virtual ~Timer() = default;
};
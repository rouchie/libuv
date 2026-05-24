#pragma once

#include "timer.h"
#include "eventpoller.h"

class UVTimer : public Timer, public std::enable_shared_from_this<UVTimer> {
public:
    using Ptr = std::shared_ptr<UVTimer>;

    explicit UVTimer(const EventPoller::Ptr &poller, const LoopContext::Ptr &loop, uint64_t timeout, uint64_t repeat, const TimerTask &task);
    ~UVTimer() override;

private:
    std::weak_ptr<EventPoller> _weakPoller;
    uv_timer_t *_timer;
};

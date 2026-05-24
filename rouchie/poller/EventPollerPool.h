#pragma once

#include "eventpoller.h"

class EventPollerPool : public std::enable_shared_from_this<EventPollerPool> {
public:
    using Ptr = std::shared_ptr<EventPollerPool>;

    static void SetPoolSize(size_t size);
    static EventPoller::Ptr GetPoller();

    ~EventPollerPool() = default;

private:
    EventPollerPool();
};

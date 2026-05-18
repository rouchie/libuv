#pragma once

#include <spdlog/common.h>

#include "taskpool.h"

class NetServer {
public:
    using Ptr = std::shared_ptr<NetServer>;

    explicit NetServer(const EventPoller::Ptr &poller = nullptr);
    virtual ~NetServer() = default;

protected:
    EventPoller::Ptr _poller;
};

inline NetServer::NetServer(const EventPoller::Ptr &poller) {
    if (!poller) {
        // 从任务池获取 TaskExecutor，然后转换为 EventPoller
        const auto taskExecutor = TaskExecutorPool::Instance().GetExecutor();
        _poller = std::dynamic_pointer_cast<EventPoller>(taskExecutor);
        
        if (!_poller) {
            throw std::runtime_error("Failed to cast TaskExecutor to EventPoller");
        }
    } else {
        _poller = poller;
    }
}

class Session {
public:
    using Ptr = std::shared_ptr<Session>;

    explicit Session(const EventPoller::Ptr &poller = nullptr);
    virtual ~Session() = default;

protected:
    EventPoller::Ptr _poller;
};

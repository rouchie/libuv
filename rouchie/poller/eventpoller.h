#pragma once

#include "taskscheduler.h"
#include "netscheduler.h"
#include "asynccontext.h"

class EventPoller : public TaskScheduler, public NetScheduler, public std::enable_shared_from_this<EventPoller> {
public:
    using Ptr = std::shared_ptr<EventPoller>;

    static Ptr Create();
    ~EventPoller() override;

    void Async(const Task& task) override;
    void FirstAsync(const Task& task) override;

    NET::Ptr TcpServer(int port, const std::string &ip = "0.0.0.0", int backlog = 1024) override;

private:
    explicit EventPoller(std::string  pollerName);

private:
    LoopContext::Ptr _loop;
    AsyncContext::Ptr _async;

    std::string _pollerName;
};

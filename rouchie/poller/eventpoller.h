#pragma once

#include "net/basesession.h"
#include "taskscheduler.h"
#include "netscheduler.h"
#include "asynccontext.h"
#include "timer.h"

class EventPoller : public TaskScheduler, public std::enable_shared_from_this<EventPoller> {
public:
    using Ptr = std::shared_ptr<EventPoller>;
    using CreateSessionFunc = std::function<BaseSession::Ptr(Ptr poller)>;

    static Ptr Create();
    ~EventPoller() override;

    void Async(const Task& task) override;
    void FirstAsync(const Task& task) override;

    void Sync(const Task &task) override;
    void FirstSync(const Task& task) override;

    NET::Ptr TcpServer(int port, const std::string &ip, int backlog, CreateSessionFunc func);

    Timer::Ptr Timer(uint64_t timeout, uint64_t repeat, const TimerTask &task);

private:
    explicit EventPoller(std::string  pollerName);

private:
    LoopContext::Ptr _loop;
    AsyncContext::Ptr _async;

    std::string _pollerName;
};


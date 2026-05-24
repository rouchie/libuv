#include "eventpoller.h"
#include "tcpserver.h"
#include "timerimp.h"

#include "spdlog/spdlog.h"

EventPoller::Ptr EventPoller::Create() {
    const auto loop = std::make_shared<LoopContext>();
    const auto async = std::make_shared<AsyncContext>(loop);

    loop->Start();

    const auto poller = std::shared_ptr<EventPoller>(new EventPoller(loop->Name()));

    poller->_loop = loop;
    poller->_async = async;

    return poller;
}

EventPoller::~EventPoller() {
    auto f = [this]() {
        _async.reset();
        _loop->Stop();
    };

    if (!_loop->IsLoopThread()) {
        // 异步删除
        Sync(f);
    } else {
        // 同步删除
        f();
    }
    SPDLOG_INFO("~EventPoller: {}", _pollerName);
}

void EventPoller::Async(const Task &task) {
    _async->Async(task);
}

void EventPoller::FirstAsync(const Task &task) {
    _async->FirstAsync(task);
}

void EventPoller::Sync(const Task &task) {
    if (_loop->IsLoopThread()) {
        task();
    } else {
        TaskScheduler::Sync(task);
    }
}

void EventPoller::FirstSync(const Task &task) {
    if (_loop->IsLoopThread()) {
        task();
    } else {
        TaskScheduler::FirstSync(task);
    }
}

NET::Ptr EventPoller::TcpServer(int port, const std::string &ip, int backlog, CreateSessionFunc func) {
    return UVTcpServer::Create(shared_from_this(), _loop, port, ip, backlog, func);
}

Timer::Ptr EventPoller::Timer(uint64_t timeout, uint64_t repeat, const TimerTask &task) {
    return std::make_shared<UVTimer>(shared_from_this(), _loop, timeout, repeat, task);
}

EventPoller::EventPoller(std::string pollerName) : _pollerName(std::move(pollerName)) {
    SPDLOG_INFO("EventPoller: {}", _pollerName);
}

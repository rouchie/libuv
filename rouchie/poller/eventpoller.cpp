#include "eventpoller.h"

#include <spdlog/spdlog.h>

#include <utility>

EventPoller::Ptr EventPoller::Create() {
    const auto loop = std::make_shared<LoopContext>();
    const auto async = std::make_shared<AsyncContext>(loop);

    loop->Start();

    const auto pollerName = fmt::format("{}", fmt::ptr(loop->Handle()));
    const auto poller = std::shared_ptr<EventPoller>(new EventPoller(pollerName));

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

void EventPoller::Async(const Task& task) {
    _async->Async(task);
}

void EventPoller::FirstAsync(const Task &task) {
    _async->FirstAsync(task);
}

NET::Ptr EventPoller::TcpServer(int port, const std::string &ip, int backlog) {
    return std::make_shared<NET>();
}

EventPoller::EventPoller(std::string pollerName) : _pollerName(std::move(pollerName)) {
    SPDLOG_INFO("EventPoller: {}", _pollerName);
}

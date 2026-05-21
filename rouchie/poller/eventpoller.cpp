#include "eventpoller.h"



EventPoller::Ptr EventPoller::Create() {
    const auto poller = std::shared_ptr<EventPoller>(new EventPoller());
    return poller;
}

void EventPoller::Async(const Task& task) {

}

void EventPoller::FirstAsync(const Task &task) {
}

NET::Ptr EventPoller::TcpServer(int port, const std::string &ip, int backlog) {
    return std::make_shared<NET>();
}

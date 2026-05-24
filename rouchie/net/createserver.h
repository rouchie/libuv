#pragma once

#include "basesession.h"
#include "poller/eventpoller.h"

template <typename SESSION>
NET::Ptr TcpServer(int port, const std::string &ip = "::", int backlog = 1024, const EventPoller::Ptr &poller = nullptr) {
    const auto f = [](EventPoller::Ptr poller) -> BaseSession::Ptr {
        return std::make_shared<SESSION>(std::move(poller));
    };

    return poller->TcpServer(port, ip, backlog, f);
}

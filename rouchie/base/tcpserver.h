#pragma once

#include "netserver.h"
#include "netscheduler.h"

class TcpServer : public NetServer {
public:
    using Ptr = std::shared_ptr<TcpServer>;

    template <typename SESSION>
    static Ptr Create(int port, const std::string &host = "::", int backlog = 1024, const EventPoller::Ptr &poller = nullptr);

    ~TcpServer() override;

private:
    explicit TcpServer(const EventPoller::Ptr &poller = nullptr);
    void Start(int port, const std::string &host, int backlog);

private:
    NET::Ptr _server;
};

template<typename SESSION>
TcpServer::Ptr TcpServer::Create(int port, const std::string &host, int backlog, const EventPoller::Ptr &poller) {
    const auto server = std::make_shared<TcpServer>(poller);
    server->Start(port, host, backlog);
    return server;
}

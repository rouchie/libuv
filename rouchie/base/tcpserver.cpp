#include "tcpserver.h"

TcpServer::TcpServer(const EventPoller::Ptr &poller)
    : NetServer(poller) {
}

TcpServer::~TcpServer() {
    auto server = std::move(_server);

    _poller->Execute([server]() mutable {
        server.reset();
    });
}

#pragma once

#include "eventpoller.h"

class UVTcpServer : public NET, public std::enable_shared_from_this<UVTcpServer> {
public:
    static Ptr Create(EventPoller::Ptr poller, LoopContext::Ptr loop, int port, const std::string &ip, int backlog, EventPoller::CreateSessionFunc func);
    ~UVTcpServer() override;

    bool Live() const override;
    std::string Error() const override;

protected:
    explicit UVTcpServer(EventPoller::Ptr poller, LoopContext::Ptr loop, int port, const std::string &ip, int backlog, EventPoller::CreateSessionFunc func);
    void Start();

private:
    EventPoller::Ptr _poller;
    LoopContext::Ptr _loop;

    uv_tcp_t *_handle;

    bool _live = false;
    std::string _error = "not start";

    EventPoller::CreateSessionFunc _createSessionFunc;
};
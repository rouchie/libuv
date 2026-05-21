#pragma once

#include "netscheduler.h"
#include "uv.h"

class UVTcpServer : public NET {
public:
    explicit UVTcpServer(const std::shared_ptr<uv_loop_t> &_loop, int port, const std::string &ip = "0.0.0.0", int backlog = 1024);

private:
    std::string _ip;
    int _port;
    int _backlog;
};
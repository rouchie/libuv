#pragma once

#include <memory>
#include <string>

class NET {
public:
    using Ptr = std::shared_ptr<NET>;

    NET() = default;
    virtual ~NET() = default;
};

class NetScheduler {
public:
    using Ptr = std::shared_ptr<NetScheduler>;

    NetScheduler() = default;
    virtual ~NetScheduler() = default;

    virtual NET::Ptr TcpServer(int port, const std::string &ip = "0.0.0.0", int backlog = 1024) = 0;
};

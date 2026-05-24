#pragma once

#include <memory>
#include <string>
#include <utility>

class NET {
public:
    using Ptr = std::shared_ptr<NET>;

    NET(std::string ip, const int port, const int backlog) : _ip(std::move(ip)), _port(port), _backlog(backlog) {}
    virtual ~NET() = default;

    std::string Ip() const { return _ip; }
    int Port() const { return _port; }

    virtual bool Live() const = 0;
    virtual std::string Error() const = 0;

protected:
    std::string _ip;
    int _port = 0;
    int _backlog = 128;
};

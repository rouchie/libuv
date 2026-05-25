#pragma once

#include <memory>

#include "base/exception.h"
#include "base/noncopyable.h"
#include "buffer/buffer.h"

class BaseSessionWrite : public noncopyable {
public:
    using Ptr = std::shared_ptr<BaseSessionWrite>;

    BaseSessionWrite() = default;
    virtual ~BaseSessionWrite() = default;

    virtual void Write(const Buffer::Ptr &buffer) = 0;
};

class BaseSession : public noncopyable {
public:
    using Ptr = std::shared_ptr<BaseSession>;

    explicit BaseSession(BaseSessionWrite::Ptr write) : _write(std::move(write)) {}
    virtual ~BaseSession() = default;

    virtual void OnRead(const Buffer::Ptr &buffer) = 0;
    virtual void OnError(const Exception &ex) = 0;

    void Write(const Buffer::Ptr &buffer) const;

private:
    BaseSessionWrite::Ptr _write;
};

inline void BaseSession::Write(const Buffer::Ptr &buffer) const {
    _write->Write(buffer);
}


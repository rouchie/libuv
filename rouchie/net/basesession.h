#pragma once

#include <functional>
#include <memory>

#include "base/exception.h"
#include "base/noncopyable.h"
#include "buffer/buffer.h"

class BaseSession : public noncopyable {
public:
    using Ptr = std::shared_ptr<BaseSession>;

    explicit BaseSession() = default;
    virtual ~BaseSession() = default;

    virtual void OnRead(const Buffer::Ptr &buffer) = 0;
    virtual void OnError(const Exception &ex) = 0;
};

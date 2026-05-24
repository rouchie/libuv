#pragma once

#include "base/noncopyable.h"

#include <memory>

class Buffer : public noncopyable {
public:
    using Ptr = std::shared_ptr<Buffer>;

    virtual ~Buffer() = default;

    virtual char* Data() = 0;
    virtual const char* Data() const = 0;

    virtual size_t Size() const = 0;

    virtual Buffer& Append(Buffer& buf) = 0;

    Buffer& operator+=(Buffer& buf);
    Buffer& operator<<(Buffer& buf);
};

inline Buffer & Buffer::operator+=(Buffer &buf) {
    this->Append(buf);
    return *this;
}

inline Buffer & Buffer::operator<<(Buffer &buf) {
    this->Append(buf);
    return *this;
}

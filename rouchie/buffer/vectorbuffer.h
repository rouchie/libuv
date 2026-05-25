#pragma once

#include "buffer.h"

#include <vector>

class VectorBuffer : public Buffer {
public:
    VectorBuffer(const char *data, size_t size);
    explicit VectorBuffer(const std::string &data) : VectorBuffer(data.data(), data.size()) {}

    char *Data() override;
    const char *Data() const override;

    size_t Size() const override;

    Buffer& Append(Buffer& buf) override;

private:
    std::vector<char> _data;
};

inline VectorBuffer::VectorBuffer(const char *data, size_t size) {
    _data.assign(data, data + size);
}

inline char * VectorBuffer::Data() {
    return _data.data();
}

inline const char * VectorBuffer::Data() const {
    return _data.data();
}

inline size_t VectorBuffer::Size() const {
    return _data.size();
}

inline Buffer & VectorBuffer::Append(Buffer &buf) {
    const char* data = buf.Data();
    const size_t size = buf.Size();
    _data.insert(_data.end(), data, data + size);
    return *this;
}

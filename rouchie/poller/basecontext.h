#pragma once

#include "uv.h"

class BaseContext {
public:
    BaseContext() = default;
    virtual ~BaseContext() = default;
};

inline void CloseCallback(uv_handle_t *h) {
    delete static_cast<BaseContext *>(h->data);
}

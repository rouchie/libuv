#pragma once

#include "uv.h"
#include "noncopyable.h"

class Semaphore : public noncopyable {
public:
    Semaphore();
    ~Semaphore();

    void Post();
    void Wait();
    bool TryWait();

private:
    uv_sem_t _sem{};
};
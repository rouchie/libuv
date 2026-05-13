#include "sem.h"

#include <iostream>

Semaphore::Semaphore() {
    if (uv_sem_init(&_sem, 0) != 0) {
        throw std::runtime_error("uv_sem_init failed");
    }
}

Semaphore::~Semaphore() {
    uv_sem_destroy(&_sem);
}

void Semaphore::Post() {
    uv_sem_post(&_sem);
}

void Semaphore::Wait() {
    uv_sem_wait(&_sem);
}

bool Semaphore::TryWait() {
    if (uv_sem_trywait(&_sem) == 0) {
        return true;
    }
    return false;
}

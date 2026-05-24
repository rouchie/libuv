#pragma once

#include <functional>

#include "noncopyable.h"
#include "spdlog/spdlog.h"

using DeferFunc = std::function<void()>;

class Defer : public noncopyable {
public:
    explicit Defer(DeferFunc f);

    ~Defer();

    Defer(Defer&& other) = delete;
    Defer& operator=(Defer&& other) = delete;

    void Cancel();

private:
    void Execute() const;

private:
    DeferFunc _func;
    bool _active;
};

inline Defer::Defer(DeferFunc f): _func(std::move(f)), _active(true) {}
inline Defer::~Defer() { Execute(); }

inline void Defer::Cancel() { _active = false; }

inline void Defer::Execute() const {
    if (_active && _func) {
        try {
            _func();
        } catch (...) {
            SPDLOG_ERROR("Defer execution error");
        }
    }
}

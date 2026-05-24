#include "timerimp.h"

class TimerContext : public BaseContext {
public:
    static uv_timer_t *Create(const TimerTask &task);

    uv_timer_t *Handle() const;
    bool RunTask() const;

protected:
    explicit TimerContext(const TimerTask &task);

private:
    std::shared_ptr<uv_timer_t> _handle;
    TimerTask _task;
};

uv_timer_t *TimerContext::Create(const TimerTask &task) {
    const auto *context = new TimerContext(task);
    return context->Handle();
}

uv_timer_s *TimerContext::Handle() const {
    return _handle.get();
}

bool TimerContext::RunTask() const {
    return _task();
}

TimerContext::TimerContext(const TimerTask &task) : _task(task) {
    _handle = std::make_shared<uv_timer_t>();
    _handle->data = this;
}

UVTimer::UVTimer(const EventPoller::Ptr &poller, const LoopContext::Ptr &loop, uint64_t timeout, uint64_t repeat, const TimerTask &task) : _weakPoller(poller), _timer(nullptr) {
    auto timer = TimerContext::Create(task);
    poller->FirstSync([timer, loop, timeout, repeat]() {
        uv_timer_init(loop->Handle(), timer);
        uv_unref(reinterpret_cast<uv_handle_t *>(timer));

        uv_timer_start(timer, [](uv_timer_t *handle) {
            const auto *context = static_cast<TimerContext *>(handle->data);
            if (!context->RunTask()) {
                uv_timer_stop(handle);
            }
        }, timeout, repeat);
    });
    _timer = timer;
}

UVTimer::~UVTimer() {
    const auto poller = _weakPoller.lock();
    if (poller) {
        auto timer =  _timer;
        poller->FirstSync([timer]() {
            uv_close(reinterpret_cast<uv_handle_t *>(timer), CloseCallback);
        });
    }
}

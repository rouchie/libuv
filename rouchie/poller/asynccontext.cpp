#include "asynccontext.h"

#include "spdlog/spdlog.h"

class AContext : public BaseContext {
public:
    explicit AContext(AsyncContext* async) : _data(async) {
        _ctx = std::make_shared<uv_async_t>();
        _ctx->data = this;
    }
    ~AContext() override = default;

    uv_async_t* Handle() const {
        return _ctx.get();
    }

    AsyncContext* Data() const {
        return _data;
    }

private:
    std::shared_ptr<uv_async_t> _ctx;
    AsyncContext* _data;
};

AsyncContext::AsyncContext(const LoopContext::Ptr &loop) : _loop(loop) {
    // 这里会在析构中调用 uv_close 时释放
    const auto ctx = new AContext(this);

    uv_async_init(loop->Handle(), ctx->Handle(), [](uv_async_t *async) {
        const auto* c = static_cast<AContext*>(async->data);
        const auto self = static_cast<AsyncContext*>(c->Data());
        self->AsyncCallback();
    });

    _ctx = ctx;
    SPDLOG_INFO("AsyncContext: {}", _loop->Name());
}

AsyncContext::~AsyncContext() {
    const auto* ctx = dynamic_cast<AContext*>(_ctx);
    uv_close(reinterpret_cast<uv_handle_t *>(ctx->Handle()), CloseCallback);

    SPDLOG_INFO("~AsyncContext: {}", _loop->Name());
}

void AsyncContext::Async(const Task &task) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _tasks.push_back(task);
    }
    const auto* ctx = dynamic_cast<AContext*>(_ctx);
    uv_async_send(ctx->Handle());
}

void AsyncContext::FirstAsync(const Task &task) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _tasks.push_front(task);
    }
    const auto* ctx = dynamic_cast<AContext*>(_ctx);
    uv_async_send(ctx->Handle());
}

void AsyncContext::AsyncCallback() {
    std::list<Task> tasks;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _tasks.swap(tasks);
    }

    for (auto &task : tasks) {
        task();
    }
}

#include "base/defer.h"
#include "base/sem.h"
#include "net/createserver.h"
#include "poller/eventpoller.h"
#include "spdlog/spdlog.h"

class RtspSession : public BaseSession {
public:
    using Ptr = std::shared_ptr<RtspSession>;

    explicit RtspSession(EventPoller::Ptr poller) : _poller(std::move(poller)) {
        _tHeartbeat = _poller->Timer(1000, 2000, []() {
            SPDLOG_INFO("RtspSession timer");
            return true;
        });
        SPDLOG_INFO("RtspSession");
    }

    ~RtspSession() override {
        SPDLOG_INFO("~RtspSession");
    }

    void OnRead(const Buffer::Ptr &buffer) override {
        SPDLOG_INFO("OnRead: {}", buffer->Size());
        if (buffer->Size() > 10) {
            throw Exception("rtsp close", CODE_SHUTDOWN);
        }
    }

    void OnError(const Exception &ex) override {
        SPDLOG_ERROR("OnError: {}", ex.what());
    }

private:
    EventPoller::Ptr _poller;
    Timer::Ptr _tHeartbeat;
};

static Defer a([]() {
    spdlog::shutdown();
});

int main() {
    const auto poller = EventPoller::Create();
    poller->Async([]() {
        SPDLOG_INFO("async ...");
    });

    auto rtsp = TcpServer<RtspSession>(50554, "::", 1024, poller);

    static Semaphore sem;
    signal(SIGINT, [](int) {
        sem.Post();
    });
    sem.Wait();

    SPDLOG_INFO("done");

    return 0;
}

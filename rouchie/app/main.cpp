#include <sstream>
#include <thread>

#include "base/sem.h"
#include "poller/eventpoller.h"

#include "spdlog/spdlog.h"

int main() {
    const auto poller = EventPoller::Create();
    poller->Async([]() {
        SPDLOG_INFO("async ...");
    });

    static Semaphore sem;
    signal(SIGINT, [](int) {
        sem.Post();
    });
    sem.Wait();

    SPDLOG_INFO("done");

    return 0;
}

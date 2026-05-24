#include "EventPollerPool.h"

#include <vector>
#include <atomic>

static size_t g_poolSize = std::thread::hardware_concurrency();
static std::vector<EventPoller::Ptr> g_poller;  // executor 池
static std::atomic<size_t> g_nextIndex{0};

void EventPollerPool::SetPoolSize(const size_t size) {
    const auto cpus = std::thread::hardware_concurrency();

    if (size == 0) {
        g_poolSize = cpus;
    } else if (cpus * 2 < size) {
        g_poolSize = cpus * 2;
    } else if (size < cpus / 2) {
        g_poolSize = cpus / 2;
    } else {
        g_poolSize = size;
    }
}

EventPoller::Ptr EventPollerPool::GetPoller() {
    static EventPollerPool instance;
    return g_poller[g_nextIndex++ % g_poolSize];
}

EventPollerPool::EventPollerPool() {
    for (size_t i = 0; i < g_poolSize; ++i) {
        g_poller.emplace_back(EventPoller::Create());
    }
}

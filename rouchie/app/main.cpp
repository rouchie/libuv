#include <sstream>
#include <thread>

#include "base/taskpool.h"
#include "base/sem.h"
#include "base/tcpserver.h"

#include "spdlog/spdlog.h"

int main() {
    const auto rtsp = std::make_shared<TcpServer>();
    rtsp->Start<int>(554);

    static Semaphore sem;
    signal(SIGINT, [](int) {
        sem.Post();
    });
    sem.Wait();

    SPDLOG_INFO("done");

    return 0;
}

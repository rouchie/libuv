#include <iostream>
#include <sstream>

#include "uv.h"
#include "base/task.h"
#include "base/sem.h"

#include "spdlog/spdlog.h"

int main() {
    for (int i= 0; i < 10; ++i) {
        auto executor = TaskExecutorImp::Create();

        new std::thread([executor]() mutable {
            for (int i = 0; i < 10; ++i) {
                executor->Execute([i, executor]() mutable {
                    // uv_sleep(1000);
                    auto id = std::this_thread::get_id();
                    executor.reset();
                    std::stringstream oss;
                    oss << id;
                    SPDLOG_INFO("task: {} : {}", i, oss.str());
                });
                executor->FirstExecute([i]() {
                    SPDLOG_INFO("first task: {}", i);
                });
                uv_sleep(100);
            }
        });

        executor.reset();
        // t.join();
    }

    static Semaphore sem;
    signal(SIGINT, [](int) { sem.Post(); }); // 设置退出信号
    sem.Wait();

    SPDLOG_INFO("done");

    return 0;
}

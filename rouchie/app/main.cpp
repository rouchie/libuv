#include <iostream>

#include "uv.h"
#include "base/task.h"
#include "base/sem.h"

int main() {
    const auto executor = TaskExecutorImp::Create();
    for (int i = 0; i < 10; ++i) {
        executor->Execute([i]() {
            std::cout << "task:" << i << std::endl;
        });
        executor->FirstExecute([i]() {
            std::cout << "first task:" << i << std::endl;
        });
        uv_sleep(100);
    }

    static Semaphore sem;
    signal(SIGINT, [](int) { sem.Post(); }); // 设置退出信号
    sem.Wait();

    return 0;
}
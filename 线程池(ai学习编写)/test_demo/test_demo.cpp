#include <iostream>
#include <thread>
#include <chrono>

#include "thread_pool.h"

/*
 * 模拟耗时任务
 * 用于测试线程池的并发执行能力
 */
int simulateHeavyTask(int taskId, int sleepMs) {
    std::cout << "Task " << taskId << " started on thread "
        << std::this_thread::get_id() << "\n";

    // 模拟耗时操作
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));

    std::cout << "Task " << taskId << " finished\n";
    return taskId * 10;
}

/*
 * 打印当前线程信息
 */
void printCurrentThread(const char* prefix) {
    std::cout << prefix << " on main thread "
        << std::this_thread::get_id() << "\n";
}

int main() {
    std::cout << "=== ThreadPool Demo ===\n\n";

    // 1. 创建线程池，4个工作线程
    std::cout << "1. Creating thread pool with 4 threads...\n";
    Thread_Pool pool(4);
    std::cout << "   Thread pool created!\n\n";

    // 2. 提交无返回值任务（lambda）
    std::cout << "2. Submitting void task (lambda)...\n";
    {
        auto future = pool.submit([]() {
            std::cout << "   Void task executed!\n";
            });
        future.get();  // 等待完成
        std::cout << "   Void task done!\n\n";
    }

    // 3. 提交有返回值任务
    std::cout << "3. Submitting task with return value...\n";
    {
        auto future = pool.submit([]() {
            return 42;
            });
        int result = future.get();  // 阻塞等待结果
        std::cout << "   Result: " << result << "\n\n";
    }

    // 4. 提交多个任务，观察并发
    std::cout << "4. Submitting multiple tasks (concurrent)...\n";
    {
        std::vector<std::future<int>> futures;

        // 提交5个任务
        for (int i = 0; i < 5; ++i) {
            int taskId = i;
            auto future = pool.submit([taskId]() {
                return simulateHeavyTask(taskId, 500);
                });
            futures.push_back(std::move(future));
        }

        // 等待所有任务完成
        std::cout << "   Waiting for all tasks...\n";
        for (auto& f : futures) {
            int result = f.get();
            std::cout << "   Got result: " << result << "\n";
        }
        std::cout << "   All tasks done!\n\n";
    }

    // 5. 测试 Future
    std::cout << "5. Testing future get()...\n";
    {
        auto f1 = pool.submit([]() { return 100; });
        auto f2 = pool.submit([]() { return 200; });

        // 可以按任意顺序获取
        std::cout << "   f2 = " << f2.get() << "\n";
        std::cout << "   f1 = " << f1.get() << "\n\n";
    }

    // 6. 关闭线程池
    std::cout << "6. Shutting down thread pool...\n";
    pool.shutdown();
    std::cout << "   Thread pool shut down!\n";

    std::cout << "\n=== Demo Complete ===\n";
    return 0;
}
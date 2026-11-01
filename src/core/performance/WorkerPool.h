
#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <exception>
#include <mutex>
#include <thread>
#include <vector>

class WorkerPool {
public:
    template <typename Function>
    static void parallelFor(
        std::size_t taskCount,
        unsigned int workerCount,
        Function function
    ) {
        if (taskCount == 0) {
            return;
        }

        workerCount = std::max(1U, workerCount);

        workerCount = std::min(
            workerCount,
            static_cast<unsigned int>(taskCount)
        );

        std::atomic<std::size_t> nextTask{0};
        std::atomic<bool> stop{false};

        std::exception_ptr firstException;
        std::mutex exceptionMutex;

        auto worker = [&]() {
            while (!stop.load()) {
                const std::size_t taskIndex =
                    nextTask.fetch_add(1);

                if (taskIndex >= taskCount) {
                    break;
                }

                try {
                    function(taskIndex);
                }
                catch (...) {
                    {
                        std::lock_guard<std::mutex> lock(exceptionMutex);

                        if (!firstException) {
                            firstException = std::current_exception();
                        }
                    }

                    stop.store(true);
                    break;
                }
            }
        };

        std::vector<std::thread> threads;
        threads.reserve(workerCount);

        for (unsigned int i = 0; i < workerCount; i++) {
            threads.emplace_back(worker);
        }

        for (std::thread& thread : threads) {
            thread.join();
        }

        if (firstException) {
            std::rethrow_exception(firstException);
        }
    }
};

#pragma once

#include <cstddef>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace blamtracer {

class thread_pool {
public:
    using task_type = std::function<void()>;

    thread_pool(std::size_t thread_count);
    thread_pool() : thread_pool(1) { }
    ~thread_pool() noexcept { shutdown(); }

    void join();

    void shutdown();

    thread_pool& push_task(task_type task);

    thread_pool& push_tasks(std::size_t n, std::function<task_type()> generator);

    bool is_idle() const noexcept;

private:
    static void worker_implementation(thread_pool& pool);

    std::mutex              mutex;
    std::atomic<bool>       do_stop;
    std::condition_variable tasks_cv;
    std::queue<task_type>   tasks;

    std::atomic<std::size_t> idle_count;
    std::condition_variable  fully_idle_cv;

    std::vector<std::thread> workers;
};

} // namespace blamtracer

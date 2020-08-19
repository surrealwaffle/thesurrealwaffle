#include "thread_pool.hpp"

namespace blamtracer {

thread_pool::thread_pool(std::size_t thread_count)
    : mutex()
    , do_stop(ATOMIC_VAR_INIT(false))
    , tasks_cv()
    , tasks()
    , idle_count(ATOMIC_VAR_INIT(static_cast<std::size_t>(0)))
    , fully_idle_cv()
    , workers()
{
    thread_count = std::max(std::size_t(1), thread_count);
    workers.reserve(thread_count);

    for (; thread_count; --thread_count)
        workers.emplace_back(worker_implementation, std::ref(*this));
}

thread_pool& thread_pool::push_task(task_type task)
{
    {
        std::unique_lock lk(mutex);
        tasks.push(std::move(task));
    }
    tasks_cv.notify_one();
    return *this;
}

thread_pool& thread_pool::push_tasks(std::size_t n, std::function<task_type()> generator)
{
    {
        std::unique_lock lk(mutex);
        for (; n; --n)
            tasks.push(generator());
    }
    tasks_cv.notify_all();
    return *this;
}

void thread_pool::join()
{
    std::unique_lock lk(mutex);
    fully_idle_cv.wait(lk, [this] { return tasks.empty() && is_idle(); });
}

void thread_pool::shutdown()
{
    do_stop.store(true, std::memory_order_relaxed);
    tasks_cv.notify_all();
    for (auto& thread : workers)
        thread.join();
    workers.clear();
}

bool thread_pool::is_idle() const noexcept
{
    return idle_count.load(std::memory_order_acquire) == workers.size();
}

void thread_pool::worker_implementation(thread_pool& pool)
{
    bool is_idle = false;
    auto add_as_idle = [&pool, &is_idle] {
        const bool added = !is_idle;
        is_idle = true;
        if (!added)
            return;

        const std::size_t amount_idle = added + pool.idle_count.fetch_add(added, std::memory_order_acq_rel);
        if (amount_idle == pool.workers.size())
            pool.fully_idle_cv.notify_all();
    };

    auto remove_from_idle = [&pool, &is_idle] {
       const bool removed = is_idle;
       is_idle = false;
       pool.idle_count.fetch_sub(removed, std::memory_order_relaxed);
    };

    struct task_info {
        bool stop_signal;
        std::optional<task_type> task;

        inline constexpr operator bool() const noexcept { return !stop_signal; }
        inline constexpr void operator()() { if (task) task.value()(); }
    };

    auto get_task = [&] () -> task_info {
        std::unique_lock lk(pool.mutex);
        if (pool.tasks.empty())
            add_as_idle();
        pool.tasks_cv.wait(lk, [&] { return pool.do_stop.load(std::memory_order_relaxed) || !pool.tasks.empty(); });

        if (pool.do_stop.load(std::memory_order_acq_rel)) {
            add_as_idle();
            return {true, std::nullopt};
        } else if (pool.tasks.empty()) {
            add_as_idle();
            return {false, std::nullopt};
        }

        remove_from_idle();
        task_type task = pool.tasks.front();
        pool.tasks.pop();
        return {false, std::move(task)};
    };

    while (auto task = get_task())
        task();
}

} // namespace blamtracer

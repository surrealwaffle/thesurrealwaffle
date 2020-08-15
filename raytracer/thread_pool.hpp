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

/** \brief A collection of threads that splits the work of submitted tasks.
 */
class thread_pool {
public:
    /** \brief The unit of work submitted to the thread pool.
     */
    using task_type = std::function<void()>;

    /** \brief Constructs a thread pool with \a thread_count threads.
     */
    thread_pool(std::size_t thread_count);

    /** \brief Constructs a thread pool with one thread.
     */
    thread_pool() : thread_pool(1) { }

    /** \brief Signals the worker threads to stop and waits for them to finish
     *         execution before destroying pool resources.
     *
     * Claimed tasks will be finished, but unclaimed tasks may be unprocessed when
     * this function returns.
     */
    ~thread_pool() noexcept { shutdown(); }

    thread_pool(const thread_pool&)            = delete; ///< DELETED
    thread_pool(thread_pool&&)                 = delete; ///< DELETED
    thread_pool& operator=(const thread_pool&) = delete; ///< DELETED
    thread_pool& operator=(thread_pool&&)      = delete; ///< DELETED

    /** \brief Blocks the current thread until all tasks have been completed.
     */
    void join();

    /** \brief Signals the worker threads to stop and blocks the entire pool is idle.
     *
     * Claimed tasks will be finished, but unclaimed tasks may be unprocessed when
     * this function returns.
     */
    void shutdown();

    /** \brief Gives the thread pool a single task to complete.
     *
     * #push_task acquires a mutex to modify the task list. If the user is submitting
     * more than one task at a time, it is recommended to use #push_tasks, which
     * acquires the mutex only once when submitting the tasks.
     *
     * \return `*this`
     */
    thread_pool& push_task(task_type task);

    /** \brief Submits \a n tasks as supplied by the expression `generator()`.
     *
     * \return `*this`
     */
    thread_pool& push_tasks(std::size_t n, std::function<task_type()> generator);

    /** \brief Indicates if all the threads belonging to the thread pool are idle.
     *
     * \return `true` if and only if the entire thread pool is idle.
     */
    bool is_idle() const noexcept;

private:
    static void worker_implementation(thread_pool& pool); ///< The entry point for
                                                          ///< the worker threads.

    std::mutex              mutex;    ///< The mutex for the condition variables
                                      ///< and task list.
    std::atomic<bool>       do_stop;  ///< An indicator for the threads to stop.
    std::condition_variable tasks_cv; ///< A CV that is notified when there is a new
                                      ///< task in #tasks.
    std::queue<task_type>   tasks;    ///< The list of tasks to process.

    std::atomic<std::size_t> idle_count;    ///< The number of threads that are in a
                                            ///< waiting or finished state.
    std::condition_variable  fully_idle_cv; ///< A CV that is notified when the last
                                            ///< active thread becomes idle.

    std::vector<std::thread> workers; ///< The collection of worker threads.
};

} // namespace blamtracer

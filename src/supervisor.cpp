#include "supervisor.h"

namespace tp::details {

void supervisor::run() {
    while (!m_stop) {
        try {
            {
                std::unique_lock<std::mutex> lock(m_spv_mutex);
                for (auto *batch : m_batches) {
                    size_t num_tasks = batch->num_tasks();
                    size_t num_workers = batch->num_workers();

                    if (num_tasks > 0) {
                        // 不超过 max_workers
                        // ，尽可能多的开启运行剩余tasks的线程
                        size_t n_worker_to_add = std::min(
                            m_max_workers - num_workers,
                            num_tasks - num_workers);
                        for (size_t i = 0; i < n_worker_to_add; ++i) {
                            batch->add_worker();
                        }
                    } else if (
                        num_workers
                        > m_min_workers) { // 没有任务时，保持最低数量
                                           // worker
                        batch->del_worker();
                    }
                }
                if (!m_stop) {
                    m_spv_cv.wait_for(
                        lock, std::chrono::milliseconds(m_timeout));
                }
            }
            m_tick_callback();
        } catch (const std::exception &e) {
            std::cerr << "threadbatch: thread[" << std::this_thread::get_id()
                      << "] caught exception:\n  what(): " << e.what() << '\n';
        }
    }
}

// add threadbatch to supervised
void supervisor::supervise(threadbatch &batch) {
    // lock so that the member variables only can be modified before
    // supervisor::run thread begin or  supervisor::run thread is waiting for
    // m_spv_cv.
    std::lock_guard<std::mutex> lock(m_spv_mutex);
    m_batches.emplace_back(&batch);
}

// suspend timeout milliseconds, default timeout is max value of unsigned int.
void supervisor::suspend(unsigned int timeout) {
    std::lock_guard<std::mutex> lock(m_spv_mutex);
    m_timeout = timeout;
}

void supervisor::proceed() {
    {
        std::lock_guard<std::mutex> lock(m_spv_mutex);
        m_timeout = m_time_reset;
    }
    m_spv_cv.notify_one();
}

void supervisor::set_tick_callback(const tick_callback_t &callback) {
    m_tick_callback = callback;
}

} // namespace tp::details
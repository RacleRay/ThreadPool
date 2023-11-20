#include "threadbatch.h"

namespace tp::details {

// wait 所有 tasks 结束
threadbatch::~threadbatch() {
    std::unique_lock<std::mutex> lock(m_workers_mutex);
    m_n_to_release = m_workers.size();
    is_destructing = true;
    m_thread_cv.wait(lock, [this] { return m_n_to_release == 0; });
}

// thread 的主循环，处理任务
void threadbatch::mission() {
    std::function<void()> task;

    while (true) {
        if (m_n_to_release <= 0 && m_taskqueue.try_pop(task)) {
            task();
        } else if (m_n_to_release > 0) {
            std::lock_guard<std::mutex> lock(m_workers_mutex);
            if (m_n_to_release > 0 && m_n_to_release--) {
                m_workers.erase(std::this_thread::get_id()); // 释放当前线程
                if (is_wait_task_done) {
                    m_task_done_cv.notify_one(); // 通知等待的任务，此时线程已经销毁
                    return;
                }
                if (is_destructing) {
                    m_thread_cv.notify_one(); // 通知析构中的条件等待
                    return;   // ！！！ 这个 return 非常重要，如果函数退出不够及时，
                          // 那么可能导致对象析构之前，还没有退出 mission 的执行，导致段错误
                }
            }
        } else { // 任务队列中没有任务，且不需要析构线程
            if (is_wait_task_done) { // 表示当前 threadpatch 正在工作，等待当前
                                     // threadpatch 中的任务都完成
                std::unique_lock<std::mutex> lock(m_workers_mutex);
                m_n_done_workers++;
                m_task_done_cv.notify_one();

                m_thread_cv.wait(lock);
                // wait 防止循环空转，当没有任务时，就不再循环了，同时等待
                // threadpatch 中其他线程结束，或者超时
            } else {
                // 表示当前 threadpatch 已经完成了所有工作，此时让出 CPU 供其他
                // threadpatch 使用
                std::this_thread::yield();
            }
        }
    }
}

void threadbatch::add_worker() {
    std::lock_guard<std::mutex> lock(m_workers_mutex);
    std::thread t(&threadbatch::mission, this);
    m_workers.emplace(t.get_id(), std::move(t));
}

// lazy excution
void threadbatch::del_worker() {
    std::lock_guard<std::mutex> lock(m_workers_mutex);
    if (!m_workers.empty()) {
        m_n_to_release++;
        return;
    }
    throw std::runtime_error("threadpatch::no worker to delete");
}

bool threadbatch::wait_tasks(unsigned int timeout_ms) {
    bool res = false;
    {
        std::unique_lock<std::mutex> lock(m_workers_mutex);
        is_wait_task_done = true;
        res = m_task_done_cv.wait_for(
            lock, std::chrono::milliseconds(timeout_ms),
            [this] { return m_n_done_workers >= m_workers.size(); });
        m_n_done_workers = 0;
        is_wait_task_done = false;
    }
    m_thread_cv.notify_all();
    return res;
}

size_t threadbatch::num_workers() const {
    std::lock_guard<std::mutex> lock(m_workers_mutex);
    return m_workers.size();
}

size_t threadbatch::num_tasks() const {
    return m_taskqueue.length();
}

} // namespace tp::details
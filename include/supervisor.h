#pragma once

#include <cassert>
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "autothread.h"
#include "threadbatch.h"

namespace tp::details {

class supervisor : noncopyable {
private:
    using tick_callback_t = std::function<void()>;

    bool m_stop {false};

    size_t m_min_workers {0};
    size_t m_max_workers {0};

    unsigned int m_time_reset {0};  // for backup, and rest m_timeout to the initialized value.
    unsigned int m_timeout {0};    // the max timeout that supervisor wait sleep.

    tick_callback_t m_tick_callback {};

    autothread<join> m_spv_run_thread {};
    std::vector<threadbatch*> m_batches {};

    std::mutex m_spv_mutex {};
    std::condition_variable m_spv_cv {};
    
public:
    supervisor(int min_workers, int max_workers, unsigned int time_interval = 500)
        : m_min_workers(min_workers), m_max_workers(max_workers), 
          m_timeout(time_interval), m_time_reset(time_interval),
          m_tick_callback([] {}), m_spv_run_thread(std::thread(&supervisor::run, this)) 
    {
        assert(min_workers >= 0 && max_workers > 0 && max_workers > min_workers);
    } 

    ~supervisor() {
        {
            std::lock_guard<std::mutex> lock(m_spv_mutex);
            m_stop = true;
            m_spv_cv.notify_one();
        }
    }

    void supervise(threadbatch& batch);

    void suspend(unsigned int timeout = -1);

    void proceed();

    // tick_callback will excute every m_timeout milliseconds.
    void set_tick_callback(const tick_callback_t& callback);

private:
    /**
     * @brief run misson thread, add workers to threadbatch.
     * 
     */
    void run();
};

} // namespace tp::details


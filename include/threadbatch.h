#pragma once

#include <condition_variable>
#include <future>
#include <iostream>
#include <map>

#include "autothread.h"
#include "futureq.h"
#include "noncopyable.h"
#include "taskqueue.h"


namespace tp::details {

class threadbatch {
private:
    using worker = autothread<detach>;
    using worker_map = std::map<worker::t_id, worker>;

    size_t m_n_to_release {0};
    size_t m_n_done_workers {0};

    bool is_wait_task_done {false};
    bool is_destructing {false};

    worker_map m_workers {};
    taskqueue<std::function<void()>> m_taskqueue {};

    mutable std::mutex m_workers_mutex {};
    std::condition_variable m_thread_release_cv {};
    std::condition_variable m_task_done_cv {};


    void mission();
    
    template <typename F>
    void recursive_exec(F&& task);

    template <typename F, typename... Fs>
    void recursive_exec(F&& task, Fs&&... tasks);

public:
    explicit threadbatch(size_t n_threads = 1) {
        for (size_t i = 0; i < n_threads; ++i) {
            add_worker();
    }
    }

    ~threadbatch();

    void add_worker();
    void del_worker();  // lazy excution by monitoring m_n_to_release

    bool wait_tasks(unsigned int timeout_ms = 0);
    size_t num_workers() const;
    size_t num_tasks() const;

    // enable_if 是 SFINAE 的常见用法，当条件为真时，返回类型为 type，否则模版匹配失败，但是不会被当做错误处理
    // 在函数返回值或者参数列表中，则是被当做函数签名的一部分 

    // enable_if<to_check, type> 中 type 没写，那么默认当to_check为真时，type 是 void

    // 注册返回 void 的 normal_task
    template <typename T,
              typename F,
              typename R = details::result_of_t<F>,
              typename CHECK_R = typename std::enable_if<std::is_void<R>::value>::type>
    typename std::enable_if<std::is_same<T, normal_task>::value>::type 
    submit(F&& task) {
        m_taskqueue.push_back([task] {
            try {
                task();
            } catch (const std::exception& e) {
                std::cerr << "threadbatch: thread[" << std::this_thread::get_id() << "] caught exception:\n  what(): "<< e.what() << '\n';
            } catch (...) {
                std::cerr << "threadbatch: thread[" << std::this_thread::get_id() << "] caught unknown exception\n";
            }
        });
    }

    // 注册返回 void 的 urgent_task
    template <typename T,
              typename F,
              typename R = details::result_of_t<F>,
              typename CHECK_R = typename std::enable_if<std::is_void<R>::value>::type>
    typename std::enable_if<std::is_same<T, urgent_task>::value>::type 
    submit(F&& task) {
        m_taskqueue.push_front([task] {
            try {
                task();
            } catch (const std::exception& e) {
                std::cerr << "threadbatch: thread[" << std::this_thread::get_id() << "] caught exception:\n  what(): "<< e.what() << '\n';
            } catch (...) {
                std::cerr << "threadbatch: thread[" << std::this_thread::get_id() << "] caught unknown exception\n";
            }
        });
    }

    template <typename T,
              typename F,
              typename... Fs>
    typename std::enable_if<std::is_same<T, sequence_task>::value>::type
    submit(F&& task, Fs&&... tasks) {
        m_taskqueue.push_back([=] {
            try {
                this->recursive_exec(task, tasks...);
            } catch (const std::exception& e) {
                std::cerr << "threadbatch: thread[" << std::this_thread::get_id() << "] caught exception:\n  what(): "<< e.what() << '\n';
            } catch (...) {
                std::cerr << "threadbatch: thread[" << std::this_thread::get_id() << "] caught unknown exception\n";
            }
        });
    }

    // 不是返回 void 的 normal task，使用 future 来返回
    template <typename T,
              typename F,
              typename R = details::result_of_t<F>,
              typename CHECK_R = typename std::enable_if<!std::is_void<R>::value>::type>
    std::future<R> submit(
        F&& task, 
        typename std::enable_if<std::is_same<T, normal_task>::value, normal_task>::type _ = {}) 
    {
        std::function<R()> func(std::forward<F>(task));
        std::shared_ptr<std::promise<R>> task_promise_sp = std::make_shared<std::promise<R>>();
        m_taskqueue.push_back([func, task_promise_sp] {
            try {
                task_promise_sp->set_value(func());
            } catch (...) {
                try {
                    task_promise_sp->set_exception(std::current_exception());
                } catch (const std::exception& e) {
                    std::cerr << "threadbatch: thread[" << std::this_thread::get_id() << "] caught exception:\n  what(): "<< e.what() << '\n';
                } catch (...) {
                    std::cerr << "threadbatch: thread[" << std::this_thread::get_id() << "] caught unknown exception\n";
                }
            }
        });
        return task_promise_sp->get_future();  // get result
    }

    // 不是返回 void 的 urgent task，使用 future 来返回。向任务队列中 push front
    template<typename T,
             typename F,
             typename R = details::result_of_t<F>,
             typename CHECK_R = typename std::enable_if<!std::is_void<R>::value>::type>
    std::future<R> submit(
        F&& task,
        typename std::enable_if<std::is_same<T, urgent_task>::value, urgent_task>::type _ = {})
    {
        std::function<R()> func(std::forward<F>(task));
        std::shared_ptr<std::promise<R>> task_promise_sp = std::make_shared<std::promise<R>>();
        m_taskqueue.push_front([func, task_promise_sp] {
            try {
                task_promise_sp->set_value(func());
            } catch (...) {
                try {
                    task_promise_sp->set_exception(std::current_exception());
                } catch (const std::exception& e) {
                    std::cerr << "threadbatch: thread[" << std::this_thread::get_id() << "] caught exception:\n  what(): "<< e.what() << '\n';
                } catch (...) {
                    std::cerr << "threadbatch: thread[" << std::this_thread::get_id() << "] caught unknown exception\n";
                }
            }
        });
        return task_promise_sp->get_future();  // get result
    }  
};

}  // namespace tp::details
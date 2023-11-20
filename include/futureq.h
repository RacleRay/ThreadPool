#pragma once

#include <deque>
#include <future>
#include <vector>


namespace tp::details {

// type_traits
#if __cplusplus >= 201703L
template <typename F, typename... Args>
using result_of_t = std::invoke_result_t<F, Args...>;
#else
template <typename F, typename... Args>
using result_of_t = typename std::result_of<F(Args...)>::type;
#endif


struct normal_task {};
struct urgent_task {};
struct sequence_task {};


/**
 * @brief Manage the std::future<T>, returned by std::promise tasks
 * 
 * @tparam T 
 */
template <typename T>
class futureq {
public:
    using futqueue_t = std::deque<std::future<T>>;
    using iterator = typename futqueue_t::iterator;

    void wait() {
        for (auto& f : m_future_queue) {
            f.wait();
        }
    }

    [[nodiscard]] size_t size() const noexcept {
        return m_future_queue.size();
    }

    // get results of all futures
    std::vector<T> get() {
        std::vector<T> res;
        for (auto& f : m_future_queue) {
            res.emplace_back(f.get());
        }
        return res;
    }

    iterator& begin() const noexcept {
        return m_future_queue.begin();
    }

    iterator& end() const noexcept {
        return m_future_queue.end();
    }

    void add_back(std::future<T>&& future) {
        m_future_queue.emplace_back(std::move(future));
    }

    void add_front(std::future<T>&& future) {
        m_future_queue.emplace_front(std::move(future));
    }

    // overload for for_each
    void for_each(std::function<void(std::future<T>&)> func) {
        for (auto& f : m_future_queue) {
            func(f);
        }
    }

    //`overload for for_each, deal from first iterator
    void for_each(const iterator& first, std::function<void(std::future<T>&)> func) {
        for (auto it = first; it != m_future_queue.end(); ++it) {
            func(*it);
        }
    }

    // overload for for_each, deal from first iterator to last iterator
    void for_each(const iterator& first, const iterator& last, std::function<void(std::future<T>&)> func) {
        for (auto it = first; it != last; ++it) {
            func(*it);
        }
    }

    std::future<T>& operator[](size_t idx) const {
        return m_future_queue[idx];
    }

private:
    futqueue_t m_future_queue;
};


}  // namespace tp::details
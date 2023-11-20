#pragma once

#include <deque>
#include <mutex>

#include "noncopyable.h"


namespace tp::details {

template <typename T>
class taskqueue: noncopyable {
public:
    using size_type = typename std::deque<T>::size_type;
    
    taskqueue() = default;

    void push_back(const T& task) {
        std::lock_guard<std::mutex> lock(m_qlock);
        m_queue.push_back(task);
    }

    void push_back(T&& task) {
        std::lock_guard<std::mutex> lock(m_qlock);
        m_queue.emplace_back(std::move(task));
    }

    void push_front(const T& task) {
        std::lock_guard<std::mutex> lock(m_qlock);
        m_queue.push_front(task);
    }

    // for urgent task
    void push_front(T&& task) {
        std::lock_guard<std::mutex> lock(m_qlock);
        m_queue.emplace_front(std::move(task));
    }

    bool try_pop(T& poptask) {
        if (!m_queue.empty()) {  // double check
            std::lock_guard<std::mutex> lock(m_qlock);
            if (!m_queue.empty()) {
                poptask = std::move(m_queue.front());
                m_queue.pop_front();
                return true;
            }
        }
        return false;
    }

    size_type length() const {
        std::lock_guard<std::mutex> lock(m_qlock);
        return m_queue.size();
    }

private:
    mutable std::mutex m_qlock;
    std::deque<T> m_queue;
};

} // namespace tp::details
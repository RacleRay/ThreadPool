#pragma once

#include <thread>

#include "noncopyable.h"


namespace tp::details {

struct join {};
struct detach {};

template <typename T>
class autothread: noncopyable {};

// Only moveable
template <>
class autothread<join>: noncopyable {
    std::thread m_thread;

public:
    autothread() = default;

    explicit autothread(std::thread&& t) : m_thread(std::move(t)) {}

    ~autothread() {
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    using t_id = typename std::thread::id;
    [[nodiscard]] t_id get_id() const noexcept { return m_thread.get_id(); }
};


template <>
class autothread<detach>: noncopyable {
    std::thread m_thread;

public:
    autothread() = delete;

    explicit autothread(std::thread&& t) : m_thread(std::move(t)) {}

    ~autothread() {
        if (m_thread.joinable()) {
            m_thread.detach();
        }
    }

    using t_id = typename std::thread::id;
    [[nodiscard]] t_id get_id() const noexcept { return m_thread.get_id(); }
};

} // namespace tp::details
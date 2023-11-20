#pragma once

#include <cassert>
#include <list>
#include <map>
#include <memory>
#include <vector>

#include "threadbatch.h"
#include "noncopyable.h"
#include "supervisor.h"


namespace tp::task {
    using urgent = details::urgent_task;
    using normal = details::normal_task;
    using sequence = details::sequence_task;
}  // namespace tp::task


namespace tp {

template <typename T>
using futureq_t = details::futureq<T>;

using threadbatch_t = details::threadbatch;

using supervisor_t = details::supervisor;

}  // namespace tp


namespace tp {

class batch_id {
    threadbatch_t* batch_ptr = nullptr;
    friend class threadpool;

public:
    explicit batch_id(threadbatch_t* bptr) : batch_ptr(bptr) {}

    bool operator==(const batch_id& rhs) const {
        return batch_ptr == rhs.batch_ptr;
    }

    bool operator!=(const batch_id& rhs) const {
        return batch_ptr != rhs.batch_ptr;
    }

    // bool operator<(const batch_id& rhs) const {
    //     return batch_ptr < rhs.batch_ptr;
    // }
    // This is not member function.
    friend std::ostream& operator<<(std::ostream& os, const batch_id& id) {
        os << (uint64_t)id.batch_ptr;
        return os;
    }
};


class supervisor_id {
    supervisor_t* supervisor_ptr = nullptr;
    friend class threadpool;

public:
    explicit supervisor_id(supervisor_t* sptr) : supervisor_ptr(sptr) {}

    bool operator==(const supervisor_id& rhs) const {
        return supervisor_ptr == rhs.supervisor_ptr;
    }

    bool operator!=(const supervisor_id& rhs) const {
        return supervisor_ptr != rhs.supervisor_ptr;
    }

    friend std::ostream& operator<<(std::ostream& os, const supervisor_id& id) {
        os << (uint64_t)id.supervisor_ptr;
        return os;
    }
};


class threadpool : details::noncopyable {
private:
    using batch_list_t = std::list<std::unique_ptr<threadbatch_t>>;
    using supervisor_map_t = std::map<const supervisor_t*, std::unique_ptr<supervisor_t>>;
    using batch_iter_t = typename batch_list_t::iterator;

    batch_iter_t m_cur_batch {};
    batch_list_t m_batch_list;
    supervisor_map_t m_supervisor_map;

public:
    threadpool() = default;

    ~threadpool() {
        m_supervisor_map.clear();
        m_batch_list.clear();  // batch 在 supervisor 之后析构，防止空悬指针
    }

    // attach
    batch_id attach(threadbatch_t* batch);
    supervisor_id attach(supervisor_t* supervisor);

    // detach
    std::unique_ptr<threadbatch_t>
    detach(const batch_id& id);

    std::unique_ptr<supervisor_t>
    detach(const supervisor_id& id);

    // for_each
    void for_each(const std::function<void(threadbatch_t&)>& func);
    void for_each(const std::function<void(supervisor_t&)>& func);

    // get threadbatch reference by bid, get supervisor reference by sid
    threadbatch_t& operator[](batch_id bid);
    supervisor_t& operator[](supervisor_id sid);

    // 返回 void 的 task。默认 T 提交为 normal task，如果需要提交为 urgent task，请使用 submit<task::urgent>
    template <
        typename T = task::normal,
        typename F,
        typename R = details::result_of_t<F>,
        typename CHECK_R = typename std::enable_if<std::is_void<R>::value>::type>
    void submit(F&& task) {
        assert(m_batch_list.size() > 0);
        auto* this_batch = m_cur_batch->get();
        auto* target_batch = find_optimal_batch(m_cur_batch)->get();
        target_batch->submit<T>(std::forward<F>(task));
    }

    template <
        typename T = task::normal,
        typename F,
        typename R = details::result_of_t<F>,
        typename CHECK_R = typename std::enable_if<!std::is_void<R>::value>::type>
    std::future<R> submit(F&& task) {
        assert(m_batch_list.size() > 0);
        auto* this_batch = m_cur_batch->get();
        auto* target_batch = find_optimal_batch(m_cur_batch)->get();
        target_batch->submit<T>(std::forward<F>(task));
    }    

    template <typename T, typename F, typename... Fs>
    typename std::enable_if<std::is_same<T, task::sequence>::value>::type
    submit(F&& f, Fs&&... fs) {
        assert(m_batch_list.size() > 0);
        auto* this_batch = m_cur_batch->get();
        auto* target_batch = find_optimal_batch(m_cur_batch)->get();
        target_batch->submit<T>(std::forward<F>(f), std::forward<Fs>(fs)...);
    }

private:
    batch_iter_t find_optimal_batch(batch_iter_t& iter) {
        auto next_iter = iter;
        ++next_iter;
        if (next_iter == m_batch_list.end()) {
            next_iter = m_batch_list.begin();
        }

        size_t cur_num_tasks = iter->get()->num_tasks();
        while (next_iter->get()->num_tasks() > cur_num_tasks && next_iter != iter) {
            ++next_iter;
            if (next_iter == m_batch_list.end()) {
               next_iter = m_batch_list.begin();
            }
        }

        return next_iter;
    }
};

}  // namespace tp
#include "threadpool.h"

namespace tp {

// attach
batch_id threadpool::attach(threadbatch_t *batch) {
    assert(batch);
    m_batch_list.emplace_back(batch);
    m_cur_batch = m_batch_list.begin();
    return batch_id{batch};
}

supervisor_id threadpool::attach(supervisor_t *supervisor) {
    assert(supervisor);
    m_supervisor_map.emplace(supervisor, supervisor);
    return supervisor_id{supervisor};
}

// detach
std::unique_ptr<threadbatch_t> threadpool::detach(const batch_id &id) {
    for (auto it = m_batch_list.begin(); it != m_batch_list.end(); ++it) {
        if (it->get() == id.batch_ptr) {
            if (m_cur_batch == it) { ++m_cur_batch; }
            auto *ptr = it->release();
            m_batch_list.erase(it);
            return std::unique_ptr<threadbatch_t>(ptr);
        }
    }
    return nullptr;
}

std::unique_ptr<supervisor_t> threadpool::detach(const supervisor_id &id) {
    auto it = m_supervisor_map.find(id.supervisor_ptr);
    if (it == m_supervisor_map.end()) { return nullptr; }
    auto *ptr = it->second.release();
    m_supervisor_map.erase(it);
    return std::unique_ptr<supervisor_t>(ptr);
}

// for_each: function to each threadbatch
void threadpool::for_each(const std::function<void(threadbatch_t &)> &func) {
    for (auto &each : m_batch_list) { func(*each); }
}

// function to each supervisor
void threadpool::for_each(const std::function<void(supervisor_t &)> &func) {
    for (auto &each : m_supervisor_map) { func(*(each.second)); }
}

// get threadbatch reference by bid
threadbatch_t &threadpool::operator[](batch_id bid) {
    return *bid.batch_ptr;
}

// get supervisor reference by sid
supervisor_t &threadpool::operator[](supervisor_id sid) {
    return *sid.supervisor_ptr;
}

} // namespace tp
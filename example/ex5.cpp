#include "threadpool.h"

int main() {
    tp::threadpool spc;

    auto bid1 = spc.attach(new tp::threadbatch_t);
    auto bid2 = spc.attach(new tp::threadbatch_t);
    auto sid1 = spc.attach(new tp::supervisor_t(2, 4));
    auto sid2 = spc.attach(new tp::supervisor_t(2, 4));

    spc[sid1].supervise(spc[bid1]); // start supervising
    spc[sid2].supervise(spc[bid2]); // start supervising

    // Automatic assignment
    spc.submit([] {
        std::cout << std::this_thread::get_id() << " executed task"
                  << std::endl;
    });
    spc.submit([] {
        std::cout << std::this_thread::get_id() << " executed task"
                  << std::endl;
    });

    spc.for_each([](tp::threadbatch_t &each) {
        each.wait_tasks(1000);
    }); // timeout: 1000ms
}

#include <iostream>

#include "threadpool.h"

#define TID() std::this_thread::get_id()

int main() {
    using namespace tp;
    tp::threadpool space;
    auto b1 = space.attach(new tp::threadbatch_t(2));
    auto b2 = space.attach(new tp::threadbatch_t(2));
    auto sp = space.attach(new tp::supervisor_t(2, 4, 1000));

    if (b1 != b2)
        std::cout << "b1[" << b1 << "] != b2[" << b2 << "]" << std::endl;

    space[sp].supervise(space[b1]);
    space[sp].supervise(space[b2]);

    // nor task A and B
    space.submit([] { std::cout << TID() << " exec task A" << std::endl; });
    space.submit([] { std::cout << TID() << " exec task B" << std::endl; });

    // wait for tasks done
    space.for_each([](tp::threadbatch_t &each) { each.wait_tasks(-1); });

    // Detach one threadbatch_t and there remain one.
    auto br = space.detach(b1);
    std::cout << "threadpool still maintain: [" << b2 << "]" << std::endl;
    std::cout << "threadpool no longer maintain: [" << b1 << "]" << std::endl;

    auto &ref = space[b2];
    ref.submit<task::normal>(
        [] { std::cout << TID() << " exec task C" << std::endl; });

    // wait for tasks done
    space.for_each([](tp::threadbatch_t &each) { each.wait_tasks(-1); });
}
#include "threadpool.h"

template<typename F>
void repeat(F &&do_what, int times) {
    for (int i = 0; i < times; ++i) { do_what(); }
}

int main() {
    tp::threadpool space;
    auto br1 = space.attach(new tp::threadbatch_t());
    auto br2 = space.attach(new tp::threadbatch_t());
    auto br3 = space.attach(new tp::threadbatch_t());
    auto sp1 = space.attach(new tp::supervisor_t(2, 4));

    // init
    int count = 0;
    space[sp1].set_tick_callback(
        [&count] { count++; }); // same callback for each threadbatch_t

    // start supervising
    space[sp1].supervise(space[br1]);
    space[sp1].supervise(space[br2]);
    space[sp1].supervise(space[br3]);

    auto sleep_task = []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };

    repeat([sleep_task, &space] { space.submit(sleep_task); }, 300);
    space.for_each([](tp::threadbatch_t &each) { each.wait_tasks(-1); });
    std::cout << "tick times: " << count << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2)); // take a rest
    std::cout << "tick times: " << count << std::endl;

    repeat([sleep_task, &space] { space.submit(sleep_task); }, 300);
    space.for_each([](tp::threadbatch_t &each) { each.wait_tasks(-1); });
    std::cout << "tick times: " << count << std::endl;

    space[sp1].suspend(); // stop
    std::cout << "Paused superivsor" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(3)); // take a rest
    std::cout << "tick times: " << count << std::endl;

    std::cout << "Go on" << std::endl;
    space[sp1].proceed(); // go on

    std::this_thread::sleep_for(std::chrono::seconds(3)); // take a rest
}
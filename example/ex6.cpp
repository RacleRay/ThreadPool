#include "threadpool.h"

int main() {
    tp::futureq_t<int> futures;
    tp::threadpool spc;
    spc.attach(new tp::threadbatch_t(2));

    futures.add_back(spc.submit([] { return 1; }));
    futures.add_back(spc.submit([] { return 2; }));

    // wait all tasks done and get the results
    futures.wait();
    auto res = futures.get();
    for (auto &each : res) { std::cout << "got " << each << std::endl; }
}

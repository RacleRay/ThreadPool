#include "threadpool.h"

int main() {
    // 2 threads
    tp::threadbatch_t br(2);

    // no return
    br.submit<tp::task::normal>(
        [] { std::cout << "hello world" << std::endl; });

    // return std::future<int>
    auto result = br.submit<tp::task::normal>([] { return 2023; });
    std::cout << "Got " << result.get() << std::endl;

    // wait for tasks done (timeout: 1000 milliseconds)
    br.wait_tasks(1000);
}
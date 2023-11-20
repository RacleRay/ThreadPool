#include "threadpool.h"

int main() {
    tp::threadbatch_t br;

    br.submit<tp::task::sequence>(
        [] { std::cout << "task 1 done\n"; },
        [] { std::cout << "task 2 done\n"; },
        [] { std::cout << "task 3 done\n"; },
        [] { std::cout << "task 4 done\n"; });

    // wait for tasks done (timeout: no limit)
    br.wait_tasks(-1);
}

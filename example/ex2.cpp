#include "threadpool.h"

int main() {
    // 1 threads
    tp::threadbatch_t br;

    // normal task
    br.submit<tp::task::normal>([] { std::cout << "task B done\n"; });

    // urgent task
    br.submit<tp::task::urgent>([] { std::cout << "task A done\n"; });

    // wait for tasks done (timeout: no limit)
    br.wait_tasks(-1);
}

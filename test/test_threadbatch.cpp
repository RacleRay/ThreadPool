#include "threadpool.h"

int main() {
    tp::threadbatch_t br(2); // 2 threads

    // add a worker
    br.add_worker();
    std::cout << "workers: " << br.num_workers() << std::endl;

    // delete a worker
    br.del_worker();
    br.del_worker();
    std::cout << "workers: " << br.num_workers() << std::endl; // 1 worker

    // normal task
    br.submit<tp::task::normal>(
        [] { std::cout << "<normal>" << std::endl; }); // FIFO

    // normal task
    br.submit<tp::task::normal>([] { std::cout << "<normal>" << std::endl; });

    // urgent task, executed as soon as possible
    br.submit<tp::task::urgent>([] { std::cout << "<urgent>" << std::endl; });

    // sequence task
    br.submit<tp::task::sequence>(
        [] { std::cout << "<sequence1>" << std::endl; },
        [] { std::cout << "<sequence2>" << std::endl; },
        [] { std::cout << "<sequence3>" << std::endl; });
    // wait for tasks done
    br.wait_tasks(-1);

    // distruct -> close the threadpool
}

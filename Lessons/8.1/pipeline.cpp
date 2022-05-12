#include <condition_variable>
#include <functional>
#include <iostream>
#include <cstddef>
#include <vector>
#include <chrono>
#include <math.h>
#include <string>
#include <chrono>
#include <thread>
#include <deque>
#include <mutex>

#include "../utils/utimer.cpp"
#include "../utils/myqueue.cpp"

using namespace std::chrono_literals;


// #define PROGRESSPRINT
void drain(myqueue<int> &q, std::chrono::milliseconds msecs)
{
    std::cout << "Drain started" << std::endl;
    auto e = q.pop();

    while (e != EOS)
    {
        std::this_thread::sleep_for(msecs);
#ifdef PROGRESSPRINT
        std::cout << "received " << e << std::endl;
#endif
        e = q.pop();
    }
    return;
}

void source(myqueue<int> &q, int n, std::chrono::milliseconds msecs)
{
    for (int i = 0; i < n; i++)
    {
        std::this_thread::sleep_for(msecs);
        q.push(i);
#ifdef PROGRESSPRINT
        std::cout << "Source emitted " << i << std::endl;
#endif
    }
    q.push(EOS);
#ifdef PROGRESSPRINT
    std::cout << "sent EOS" << std::endl;
#endif
    return;
}

void genericstage(myqueue<int> &inq, myqueue<int> &outq, std::chrono::milliseconds msecs)
{
    auto e = inq.pop();

    while (e != EOS)
    {
        std::this_thread::sleep_for(msecs);
        auto res = e + 1;
        outq.push(res);
        e = inq.pop();
    }
    outq.push(EOS);
    return;
}

void pipe4stages(int m, std::chrono::milliseconds t0, std::chrono::milliseconds t1, std::chrono::milliseconds t2, std::chrono::milliseconds t3)
{
    myqueue<int> q1, q2, q3; // create the 3 queues to interconnect the four pipeline stages
    // now start the pipeline stages threads
    std::thread s1(source, std::ref(q1), m, t0);                  // source creates the input stream on first queue
    std::thread s2(genericstage, std::ref(q1), std::ref(q2), t1); // first computing stage (inc by 1)
    std::thread s3(genericstage, std::ref(q2), std::ref(q3), t2); // second computing stage (inc by 2)
    std::thread s4(drain, std::ref(q3), t3);                      // fourth stage (prints results)

    s1.join();
    s2.join();
    s3.join();
    s4.join(); // now await termination. In a pipeline, stages should terminate in order
}

auto completionTime(int m, std::chrono::milliseconds t0, std::chrono::milliseconds t1, std::chrono::milliseconds t2, std::chrono::milliseconds t3)
{
    auto ts = std::max(std::max(t0, t1), std::max(t2, t3));
    auto sumt = t0 + t1 + t2 + t3;
    auto tc = sumt + (m - 1) * ts;
    return tc;
}

int main(int argc, char *argv[])
{

    std::chrono::milliseconds temit, tinc1, tinc2, tdrain;
    long taskNo;

    if (argc == 1)
    {
        temit = 10ms;
        tinc1 = 20ms;
        tinc2 = 40ms;
        tdrain = 1ms;
        taskNo = 8;
    }
    else
    {
        temit = std::chrono::milliseconds(atoi(argv[1]));
        tinc1 = std::chrono::milliseconds(atoi(argv[2]));
        tinc2 = std::chrono::milliseconds(atoi(argv[3]));
        tdrain = std::chrono::milliseconds(atoi(argv[4]));
        taskNo = atoi(argv[5]);
    }

    std::cout << "Executing " << taskNo << " tasks (times are " << temit.count() << ", "
              << tinc1.count() << ", " << tinc2.count() << ", " << tdrain.count() << ")" << std::endl;

    long tpar;
    {
        utimer tt("Tc", &tpar);
        pipe4stages(taskNo, temit, tinc1, tinc2, tdrain);
    }

    std::cout << "Teo Tc = " << completionTime(taskNo, temit, tinc1, tinc2, tdrain).count() << std::endl;

    auto tseq = taskNo * (temit + tinc1 + tinc2 + tdrain); // we account for reading/writing tasks from/to disk in temit, tdrain
    std::cout << "Tseq = " << tseq.count() << "msecs" << std::endl;
    std::cout << "Speedup(4) = " << ((float)tseq.count()) / ((float)tpar) << std::endl;
    return (0);
}

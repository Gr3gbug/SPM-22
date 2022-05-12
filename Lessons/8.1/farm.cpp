#include <iostream>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <chrono>
#include <cstddef>
#include <math.h>
#include <string>
#include <functional>

#include <chrono>
#include <thread>
using namespace std::chrono_literals;

#include "../utils/myqueue.cpp"
#include "../utils/utimer.cpp"

// #define PROGRESSPRINT

void delay(std::chrono::milliseconds m)
{
#ifdef ACTIVEWAIT
    auto active_wait = [](std::chrono::milliseconds ms)
    {
        long msecs = ms.count();
        auto start = std::chrono::high_resolution_clock::now();
        auto end = false;
        while (!end)
        {
            auto elapsed = std::chrono::high_resolution_clock::now() - start;
            auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
            if (msec > msecs)
                end = true;
        }
        return;
    };
    active_wait(m);
#else
    std::this_thread::sleep_for(m);
#endif
    return;
}

void source2farm(std::vector<myqueue<int>> &q, int n, int nw, std::chrono::milliseconds msecs)
{
    int turn = 0;

    for (int i = 0; i < n; i++)
    {
        delay(msecs);
        q[turn].push(i);
        turn = (turn + 1) % nw;
#ifdef PROGRESSPRINT
        std::cout << "Source emitted " << i << std::endl;
#endif
    }
    for (int i = 0; i < nw; i++)
        q[i].push(EOS);
#ifdef PROGRESSPRINT
    std::cout << "sent EOS" << std::endl;
#endif
    return;
}

void drainfarm(myqueue<int> &q, int nw, std::chrono::milliseconds msecs)
{
#ifdef PROGRESSPRINT
    std::cout << "Drain started" << std::endl;
#endif
    auto e = q.pop();
    int countEos = 0;

    do
    {
        if (e == EOS)
        {
            countEos++;
            if (countEos == nw)
                break; // got last EOS: exit from loop
            else
                continue; // cot first EOS, look for the second one
        }
        delay(msecs);
#ifdef PROGRESSPRINT
        std::cout << "received " << e << std::endl;
#endif
        e = q.pop();
    } while (true);

    return;
}

void genericfarmstage(myqueue<int> &inq, myqueue<int> &outq, std::chrono::milliseconds msecs1, std::chrono::milliseconds msecs2)
{
    auto e = inq.pop();

    while (e != EOS)
    {
        delay(msecs1);
        delay(msecs2);
        auto res = e + 2;
        outq.push(res);
        e = inq.pop();
    }
    outq.push(EOS);
    return;
}

void farm(int m, int nw, std::chrono::milliseconds t0, std::chrono::milliseconds t1, std::chrono::milliseconds t2, std::chrono::milliseconds t3)
{
    myqueue<int> qdrain;             // input queue for the drain
    std::vector<myqueue<int>> q(nw); // input queues for the workers
    // now start the farm stages threads
    std::thread s0(source2farm, std::ref(q), m, nw, t0); // this is the emitter, source of the stream of tasks  ;
    std::vector<std::thread *> tids(nw);
    for (int i = 0; i < nw; i++)
        tids[i] = new std::thread(genericfarmstage, std::ref(q[i]), std::ref(qdrain), t1, t2);
    std::thread s1(drainfarm, std::ref(qdrain), nw, t3); // fourth stage (prints results)

    s0.join();
    for (int i = 0; i < nw; i++)
        tids[i]->join();
    s1.join();
}

auto completionTime4(int m, int nw,
                     std::chrono::milliseconds temit, std::chrono::milliseconds tinc1,
                     std::chrono::milliseconds tinc2, std::chrono::milliseconds tdrain)
{
    auto ts = max(max(temit, tdrain), (tinc1 + tinc2) / nw);
    auto sum = temit + tinc1 + tinc2 + tdrain;
    auto tc = sum + (m - 1) * ts;
    return tc;
}

int main(int argc, char *argv[])
{

    std::chrono::milliseconds temit, tinc1, tinc2, tdrain;
    long taskNo;
    long nw;

    if (argc == 1)
    {
        temit = 10ms;
        tinc1 = 20ms;
        tinc2 = 40ms;
        tdrain = 1ms;
        taskNo = 8;
        nw = 4;
    }
    else
    {
        temit = std::chrono::milliseconds(atoi(argv[1]));
        tinc1 = std::chrono::milliseconds(atoi(argv[2]));
        tinc2 = std::chrono::milliseconds(atoi(argv[3]));
        tdrain = std::chrono::milliseconds(atoi(argv[4]));
        taskNo = atoi(argv[5]);
        nw = atoi(argv[6]);
    }

    std::cout << "Executing " << taskNo << " tasks " << nw << " workers (times are " << temit.count() << ", "
              << tinc1.count() << ", " << tinc2.count() << ", " << tdrain.count() << ")" << std::endl;
    long tpar;
    {
        utimer tt("Tc", &tpar);
        farm(taskNo, nw, temit, tinc1, tinc2, tdrain);
    }

    std::cout << "Teo Tc = " << completionTime4(taskNo, nw, temit, tinc1, tinc2, tdrain).count() << std::endl;

    auto tseq = taskNo * (temit + tinc1 + tinc2 + tdrain); // we account for reading/writing tasks from/to disk in temit, tdrain
    std::cout << "Tseq = " << tseq.count() << "msecs" << std::endl;
    std::cout << "Speedup(" << nw + 2 << ") = " << ((float)tseq.count()) / ((float)tpar) << std::endl;
    return (0);
}
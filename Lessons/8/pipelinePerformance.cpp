#include <mutex>
#include <deque>
#include <vector>
#include <chrono>
#include <math.h>
#include <string>
#include <thread>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <functional>
#include <condition_variable>
#include "../utils/utimer.cpp"

#define EOS -1
using namespace std::chrono_literals;

template <typename T>
class myqueue
{
private:
  std::mutex              d_mutex;
  std::condition_variable d_condition;
  std::deque<T>           d_queue;
public:

  myqueue(std::string s) { std::cout << "Created " << s << " queue " << std::endl;  }
  myqueue() {}
  
  void push(T const& value) {
    {
      std::unique_lock<std::mutex> lock(d_mutex);
      d_queue.push_front(value);
    }
    this->d_condition.notify_one();
  }
  
  T pop() {
    std::unique_lock<std::mutex> lock(d_mutex);
    d_condition.wait(lock, [=]{ return !d_queue.empty(); });  // wait if the queue is currently empty
    T rc(std::move(this->d_queue.back()));       // used to get the actual message rather than a copy
    d_queue.pop_back();                   // remove the item just read from the back end of the queue
    return rc;
  }
};

void drain(myqueue<int> &q, int numberOfWorkers) {
  std::cout << "Drain started" << std::endl;
  auto e = q.pop();
  int counter{0};
  while(e != EOS && counter < numberOfWorkers) {
    std::this_thread::sleep_for(100ms);
    std::cout << "received " << e << std::endl;
    e = q.pop();
    if(e == EOS)
      counter++;
  }
    
  return;
}

void source(myqueue<int> &q, int n) {
    for(int i=0; i<n; i++){
        std::this_thread::sleep_for(100ms);
        q.push(i);
        std::cout << "Drain emitted " << i << std::endl; 
    }
    q.push(EOS);
    std::cout << "sent EOS" << std::endl;
    return;
}

void stage1(myqueue<int> &inq, myqueue<int> &outq, int numberOfWorkers) {
  auto e = inq.pop();
  
  while(e != EOS) {
    std::this_thread::sleep_for(100ms);
    auto res = e+1;
    outq.push(res);
    e = inq.pop();
    std::cout << "Added " << e << std::endl;
  }
  for(int nw=0; nw < numberOfWorkers; nw++)
  {
    outq.push(EOS); 
  }

  return;
}

void farm(myqueue<int> &inq, myqueue<int> &outq, int thID) {
  auto e = inq.pop();
  while(e != EOS) {
    std::this_thread::sleep_for(2500ms);
    auto res = e*2;
    outq.push(res);
    e = inq.pop();
    std::cout << "Multiplied " << e << " from thID: " << thID <<std::endl;
  }
  outq.push(EOS);
  return;
}

int main(int argc, char *argv[])
{
  int nw = atoi(argv[1]);

  myqueue<int> source2one, intermediate, int2drain;
  std::vector<std::thread> vecOfThread;
  
  auto inc = [](int x)
  { return (x + 1); }; // Dummy function

  std::thread s1(source, std::ref(source2one), 50);
  s1.join();

  std::thread s2(stage1, std::ref(source2one), std::ref(intermediate), nw);
  s2.join();

  for (size_t i = 0; i < 2; i++)
    vecOfThread.push_back(std::thread(farm, std::ref(intermediate), std::ref(int2drain), i));

  for (std::thread &th : vecOfThread)
    th.join();

  std::thread s4(drain, std::ref(int2drain), nw);
  s4.join();

  return 0;
}
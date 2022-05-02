#include <mutex>
#include <deque>
#include <vector>
#include <chrono>
#include <math.h>
#include <string>
#include <chrono>
#include <thread>
#include <cstddef>
#include <iostream>
#include <condition_variable>

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

void drain(myqueue<int> &q) {
  std::cout << "Drain started" << std::endl;
  auto e = q.pop();
  
  while(e != EOS) {
    std::cout << "received " << e << std::endl;
    e = q.pop();
  }
  return;
}

void source(myqueue<int> &q) {
    for(int i=0; i<16; i++){
        q.push(i);
        std::this_thread::sleep_for(1000ms);
    }
    q.push(EOS);
    std::cout << "sent EOS" << std::endl;
    return;
}

int main(int argc, char *argv[])
{
    myqueue<int> myq;
    std::thread tdrain(drain,std::ref(myq));
    std::thread tsource(source,std::ref(myq));
    tsource.join();
    tdrain.join();
    
    std::cout << "End phase one" << std::endl;

    myqueue<int> myq1;
    std::thread tsource1(source,std::ref(myq1));
    std::thread tdrain1(drain,std::ref(myq1));

    tsource1.join();
    tdrain1.join();

    return 0;
}
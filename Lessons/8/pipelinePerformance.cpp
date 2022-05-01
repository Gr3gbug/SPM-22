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

#define EOS -1

#include <chrono>
#include <thread>
using namespace std::chrono_literals;

void drain(myqueue<int> &q, std::chrono::milliseconds msecs) {
  std::cout << "Drain started" << std::endl;
  auto e = q.pop();
  
  while(e != EOS) {
    std::this_thread::sleep_for(msecs);
    std::cout << "received " << e << std::endl;
    e = q.pop();
  }
    
  return;
}

void source(myqueue<int> &q, int n, std::chrono::milliseconds msecs) {
    for(int i=0; i<n; i++){
        std::this_thread::sleep_for(msecs);
        q.push(i);
        std::cout << "Drain emitted " << i << std::endl; 
    }
    q.push(EOS);
    std::cout << "sent EOS" << std::endl;
    return;
}

void genericstage(myqueue<int> &inq, myqueue<int> &outq, std::chrono::milliseconds msecs) {
  auto e = inq.pop();
  
  while(e != EOS) {
    std::this_thread::sleep_for(msecs);
    auto res = e+1;
    outq.push(res);
    e = inq.pop();
  }
  outq.push(EOS);
  return;
}

myqueue<int> source2one, one2drain;

std::thread s1(source, std::ref(source2one), 10, 1000ms);

auto inc = [](int x) { return(x+1);};

std::thread s2(genericstage, std::ref(source2one), std::ref(one2drain), 1000ms);

std::thread s3(drain, std::ref(one2drain), 1000ms);

s1.join();
s2.join();
s3.join();
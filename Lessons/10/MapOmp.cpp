#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <vector>
#include <thread>

#include "utimer.cpp"

using namespace std;

void activewait(std::chrono::milliseconds ms) {
  long msecs = ms.count();
  auto start = std::chrono::high_resolution_clock::now();
  auto end   = false;
  while(!end) {
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    if(msec>msecs)
      end = true;
  }
  return;
}

int f(int x) {
  activewait(10ms);
  return(x+x);
}

int main(int argc, char * argv[]) {

  if(argc == 1) {
    cout << "Usage is:" << argv[0] << " seed n [pardegree]" << endl;
    return(0);
  }  
  int seed = atoi(argv[1]);
  int n = atoi(argv[2]);
  int nw = atoi(argv[3]);

  const int max = 1024;
  srand(seed);
  vector<int> v(n),r1(n),r2(n);
  for(int i=0; i<n; i++)
    v[i] = rand()%max;
  

  {
    utimer tseq("seq");
    transform(v.begin(), v.end(), r1.begin(),f);
  }

  
  {
    utimer t1("par");
#pragma omp parallel for num_threads(nw)
    for(int i=0; i<n; i++)
      r2[i] = f(v[i]);
  }
    

  for(int i=0; i<n; i++)
    if(r1[i] != r2[i]) 
      cout << r1[i] << " != " << r2[i] << " at " << i << endl;

  return(0);
}
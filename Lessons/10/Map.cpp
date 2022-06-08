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
  activewait(1ms);
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

  {                                          // pthread parallel execution
    auto compute_chunk = [&v,&r2](pair<int,int> range) {   // function to compute a chunk
      for(int i=range.first; i<range.second; i++) {
	r2[i] = f(v[i]);
      }
      return; 
    };
    
    vector<pair<int,int>> ranges(nw);                     // vector to compute the ranges 
    int delta { n / nw };
    vector<thread> tids;
    

    {
      utimer t4("split");
      for(int i=0; i<nw; i++) {                     // split the string into pieces
	ranges[i] = make_pair(i*delta,(i != (nw-1) ? (i+1)*delta : n)); 
      }
    }

    {
      utimer t1("par");
      for(int i=0; i<nw; i++) {                     // assign chuncks to threads
	tids.push_back(thread(compute_chunk, ranges[i]));
      }
      
      for(thread& t: tids) {                        // await thread termination
	t.join();
      }
    }
    
  }

  // merge is empty
  
  for(int i=0; i<n; i++)
    if(r1[i] != r2[i]) 
      cout << r1[i] << " != " << r2[i] << " at " << i << endl;

  return(0);
<<<<<<< HEAD
}
=======
}
>>>>>>> 1104b8958552623f5491c64ca307e2df5ffb9e04

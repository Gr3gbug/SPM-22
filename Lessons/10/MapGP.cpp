#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <vector>
#include <thread>

#include "grppi/pipeline.h"
#include "grppi/map.h"
#include "grppi/dyn/dynamic_execution.h"

using namespace grppi;


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
  int m = atoi(argv[3]);
  int nw = atoi(argv[4]);


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

    grppi::pipeline(parallel_execution_native{nw},
		    [&] () -> std::optional<vector<int>> {
		      static int i = 0;
		      i++;
		      if(i<m) return {vector<int>(n,i)};
		      else return{};
		    },
		    [&](vector<int> v) {
		      grppi::map(sequential_execution{},
				 // grppi::map(parallel_execution_native{nw},
				 v.begin(),v.end(), v.begin(), f);
		      return v;
		    },
		    [&] (vector<int> v) {
		      cout << "Got ";
		      for(auto &e : v)
			cout << e << " ";
		      cout << endl; 
		    }
		    );
    
    grppi::map(parallel_execution_native{nw},
	       v.begin(), v.end(), r2.begin(),
	       f);
  }
    

  for(int i=0; i<n; i++)
    if(r1[i] != r2[i]) 
      cout << r1[i] << " != " << r2[i] << " at " << i << endl;

  return(0);
<<<<<<< HEAD
}
=======
}
>>>>>>> 1104b8958552623f5491c64ca307e2df5ffb9e04

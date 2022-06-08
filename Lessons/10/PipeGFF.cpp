#include <iostream>
#include <optional>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::literals::chrono_literals;

#include "grppi/pipeline.h"
#include "grppi/farm.h"
#include "grppi/dyn/dynamic_execution.h"

using namespace grppi;

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

#include "utimer.cpp"

int inc(int i) {
  activewait(10ms);
  return(i+1);
}

int twice(int i) {
  activewait(50ms);
  return(i+i);
}

int main(int argc, char * argv[]) {

  int m,nw1,nw2;
  if (argc != 1) {
    m   = atoi(argv[1]);
    nw1 = atoi(argv[2]);
    nw2 = atoi(argv[3]);
  } else {
    m = 16;
    nw1 = 2;
    nw2 = 4;
  } 
   
  
  auto source =
    [&m] () -> std::optional<int> {
				   static int n = 0;
				   if(n == m)
				     return {};
				   else
				     return{n++};
  };

  auto drain =
    [] (int n) {
      cout << "Received " << n << endl;
    };

  {
    utimer t("pipe");

    pipeline(grppi::parallel_execution_native{},
    // pipeline(grppi::sequential_execution{},
	     source,
	     farm(nw1,inc),
	     farm(nw2,twice),
	     drain);
  }
  
  return(0);
  
<<<<<<< HEAD
}
=======
}
>>>>>>> 1104b8958552623f5491c64ca307e2df5ffb9e04

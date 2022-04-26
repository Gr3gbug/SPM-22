#include <iostream> 
#include <vector>
#include <functional>
#include <thread>

using namespace std;

int sum = 0;

int main(int argc, char * argv[]) {

  int n = atoi(argv[1]);
  int seed = atoi(argv[2]);

  const int k = 128;
  
  vector<int> x(n);

  srand(seed);
  for(int i=0; i<n; i++)
    x[i] = rand()%k;

  int seqsum = 0;
  for(int i=0; i<n; i++)
    seqsum += x[i];

  cout << "Seq sum " << seqsum << endl;

  auto f = [&](int start, int stop) {
	     for(int i=start; i<stop; i++)
	       sum += x[i];
	   };

  auto tid1 = new thread(f, 0, n/2);
  auto tid2 = new thread(f, n/2, n);
  tid1->join();
  tid2->join();
  cout << "Par sum " << sum << endl; 

  return(0);

}

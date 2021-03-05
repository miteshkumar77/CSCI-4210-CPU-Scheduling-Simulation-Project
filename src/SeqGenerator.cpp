#include "SeqGenerator.hpp"


inline double SeqGenerator::nextExp(double lambda) {
  return -log(drand48()) / lambda; 
}

inline unsigned int SeqGenerator::randNumBursts() {
  return drand48() * 100; 
}

unsigned int SeqGenerator::floorNextExp(double lambda, double maxval) {
  double res = std::numeric_limits<unsigned int>::max();
  while(res > maxval) res = floor(nextExp(lambda));
  return res; 
}

unsigned int SeqGenerator::ceilNextExp(double lambda, double maxval) {
  double res = std::numeric_limits<unsigned int>::max(); 
  while(res > maxval) res = floor(nextExp(lambda));
  return res;
}

std::vector<Process> SeqGenerator::generateProccesses(unsigned short n, double lambda, double maxval) {
  unsigned int arrivalTime;
  unsigned int numBursts;
  std::vector<Process> processes;
  processes.reserve(n); 
  for (unsigned short i = 0; i < n; ++i) {

    arrivalTime = floorNextExp(lambda, maxval);
    numBursts = randNumBursts(); 
    std::vector<unsigned int> cpuBurstTimes(numBursts);
    std::vector<unsigned int> ioBurstTimes(numBursts);
    for (unsigned int j = 0; j < numBursts; ++j) {
      cpuBurstTimes[j] = ceilNextExp(lambda, maxval);
      ioBurstTimes[j] = 10 * ceilNextExp(lambda, maxval); 
    }
    processes.push_back(Process(arrivalTime, cpuBurstTimes, ioBurstTimes)); 
  }
  return processes;
}
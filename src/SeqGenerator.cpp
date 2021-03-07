#include "SeqGenerator.hpp"


inline double SeqGenerator::nextExp(double lambda) {
  return -log(drand48()) / lambda; 
}

inline unsigned int SeqGenerator::randNumBursts() {
  return 1 + drand48() * 100; 
}

unsigned int SeqGenerator::floorNextExp(double lambda, double maxval) {
  double res = std::numeric_limits<unsigned int>::max();
  while(res > maxval) res = floor(nextExp(lambda));
  return res; 
}

unsigned int SeqGenerator::ceilNextExp(double lambda, double maxval) {
  double res = std::numeric_limits<unsigned int>::max(); 
  while(res > maxval) res = ceil(nextExp(lambda));
  return res;
}

std::vector<Process> SeqGenerator::parseProcesses(std::string fname) {
  FILE* fp;
  if (NULL == (fp = fopen(fname.c_str(), "r"))) {
    throw std::runtime_error("Error: could not open file.");
  }

  

  unsigned int n;
  unsigned int arrivalTime;
  unsigned int nCpuBursts;
  unsigned int nIoBursts;

  fscanf(fp, "%u", &n);
  std::vector<Process> processes;
  processes.reserve(n); 
  for (int i = 0; i < n; ++i) {
    fscanf(fp, "%u", &arrivalTime);
    fscanf(fp, "%u", &nCpuBursts);
    if (nCpuBursts == 0) {
      throw std::runtime_error("Error: nCpuBursts was 0.");
    }
    nIoBursts = nCpuBursts - 1;
    std::vector<unsigned int> cpuBurstTimes(nCpuBursts);
    std::vector<unsigned int> ioBurstTimes(nIoBursts);
    for (unsigned int j = 0; j < nCpuBursts; ++j) {
      fscanf(fp, "%u", &cpuBurstTimes[j]); 
    }

    for (unsigned int j = 0; j < nIoBursts; ++j) {
      fscanf(fp, "%u", &ioBurstTimes[j]); 
    }
    processes.push_back(Process(arrivalTime, cpuBurstTimes, ioBurstTimes)); 
  }

  return processes;
}

std::vector<Process> SeqGenerator::generateProccesses(unsigned short n, double lambda, double maxval, long int seedval) {
  srand48(seedval); 
  unsigned int arrivalTime;
  unsigned int numBursts;
  std::vector<Process> processes;
  processes.reserve(n); 
  for (unsigned short i = 0; i < n; ++i) {

    arrivalTime = floorNextExp(lambda, maxval);
    numBursts = randNumBursts(); 
    std::vector<unsigned int> cpuBurstTimes(numBursts);
    std::vector<unsigned int> ioBurstTimes(numBursts - 1);
    for (unsigned int j = 0; j < numBursts; ++j) {
      cpuBurstTimes[j] = ceilNextExp(lambda, maxval);
      if (j + 1 != numBursts) {
        ioBurstTimes[j] = 10 * ceilNextExp(lambda, maxval); 
      }
    }
    processes.push_back(Process(arrivalTime, cpuBurstTimes, ioBurstTimes)); 
  }
  return processes;
}
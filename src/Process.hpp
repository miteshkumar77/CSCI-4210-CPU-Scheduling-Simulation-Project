#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <vector>
#include <exception>
#include <stdexcept>

enum State : short {RUNNING, READY, WAITING}; 

class Process {

public:
Process(unsigned int arrivalTime, 
  const std::vector<unsigned int>& cpuBurstTimes, 
  const std::vector<unsigned int>& ioBurstTimes); 

private:

  static char gpid; 
  const unsigned int arrivalTime;
  const char pid;
  const std::vector<unsigned int> originalCpuBurstTimes;
  const std::vector<unsigned int> originalIoBurstTimes;
  std::vector<unsigned int> cpuBurstTimes;
  std::vector<unsigned int> ioBurstTimes;
  unsigned int burstIdx = 0;

};





#endif
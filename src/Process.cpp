#include "Process.hpp"

char Process::gpid = 'A' - 1; 

Process::Process(unsigned int arrivalTime, 
  const std::vector<unsigned int>& cpuBurstTimes, 
  const std::vector<unsigned int>& ioBurstTimes): 
  arrivalTime(arrivalTime), 
  pid(++gpid), 
  originalCpuBurstTimes(std::vector<unsigned int>(cpuBurstTimes.begin(), cpuBurstTimes.end())),
  originalIoBurstTimes(std::vector<unsigned int>(ioBurstTimes.begin(), ioBurstTimes.end())),
  cpuBurstTimes(std::move(cpuBurstTimes)),
  ioBurstTimes(std::move(ioBurstTimes)) {
  
  if (gpid > 'Z') {
    gpid = 'Z'; 
    throw std::runtime_error("Error: Process Constructor called more than 26 times...");
  }
  
}

void Process::printProcess() {
  std::cout << "pid: " << pid << std::endl;
  std::cout << "arrival time: " << arrivalTime << std::endl;
  std::cout << "current burst index: " << burstIdx << std::endl;
  std::cout << "cpu burst times: ";
  for (auto i : cpuBurstTimes) std::cout << i << ' ';
  std::cout << std::endl;
  std::cout << "io burst times: ";
  for (auto i : ioBurstTimes) std::cout << i << ' ';
  std::cout << std::endl;
  std::cout << std::endl;
}
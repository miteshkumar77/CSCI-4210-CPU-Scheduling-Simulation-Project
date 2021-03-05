#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>

enum class State: char {UNARRIVED, READY, WAITING, RUNNING, TERMINATED};

class Process {

public:
  Process(unsigned int arrivalTime, 
    const std::vector<unsigned int>& cpuBurstTimes, 
    const std::vector<unsigned int>& ioBurstTimes); 


  int getArrivalTime() const { return arrivalTime; }
  char getPid() const { return pid; }
  void printProcess();
  void reset();
  void nextState();
  void terminate(); 
private:
  static char gpid; 
  const unsigned int arrivalTime;
  const char pid;
  const std::vector<unsigned int> originalCpuBurstTimes;
  const std::vector<unsigned int> originalIoBurstTimes;
  std::vector<unsigned int> cpuBurstTimes;
  std::vector<unsigned int> ioBurstTimes;
  unsigned int burstIdx = 0;
  State processState;
};





#endif
#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>



class Process {

public:
  Process(unsigned int arrivalTime, 
    const std::vector<unsigned int>& cpuBurstTimes, 
    const std::vector<unsigned int>& ioBurstTimes); 

  enum class State: char {UNARRIVED, READY, WAITING, RUNNING, TERMINATED};

  int getArrivalTime() const { return arrivalTime; }
  unsigned int getCurrIoBurstTime() const;
  char getPid() const { return pid; }
  State decrementBurst();
  State getState() const { return processState; }
  void preempt();
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
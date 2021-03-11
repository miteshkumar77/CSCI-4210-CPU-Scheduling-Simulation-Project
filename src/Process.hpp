#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <string>
#include "globals.hpp"

class Process {

public:
  enum class State: char {UNARRIVED, READY, SW_IN, RUNNING, SW_READY, SW_WAIT, WAITING, SW_TERM, TERMINATED};
  Process(unsigned int arrivalTime, 
    const std::vector<unsigned int>& cpuBurstTimes, 
    const std::vector<unsigned int>& ioBurstTimes); 
  std::string nextState(unsigned int timestamp, unsigned int tcs); 
  Process::State getState() const { return processState; }
  Process::State decrementBurst();
  void printInfo() const;
  void preempt(); 
  int getArrivalTime() const { return arrivalTime; }
  unsigned int getCurrIoBurstTime() const;
  char getPid() const { return pid; }
private:
  void startWaitingTimer(unsigned int timestamp);
  void endWaitingTimer(unsigned int timestamp);
  void startTurnaroundTimer(unsigned int timestamp);
  void endTurnaroundTimer(unsigned int timestamp); 
  static char gpid; 
  const unsigned int arrivalTime;
  const char pid;
  const std::vector<unsigned int> originalCpuBurstTimes;
  const std::vector<unsigned int> originalIoBurstTimes;
  std::vector<unsigned int> cpuBurstTimes;
  std::vector<unsigned int> waitingTimes;
  std::vector<unsigned int> turnaroundTimes;
  signed long waitingTimer = -1;
  signed long turnaroundTimer = -1;
  unsigned int burstIdx = 0;
  State processState;
};
#endif
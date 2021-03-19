/**
 *  CSCI 4210 Operating Systems
 *  2021 Spring
 * 
 *  Simulation Project - Process.hpp
 * 
 *  Authors:
 *    Mitesh Kumar  [ kumarm4 ]
 *    Jason Lam     [  lamj7  ]
 *    William He    [  hew7   ]
 * 
 *  Brief:
 *    Represents the process objects. Only allows the creation of up to 26 processes per
 *    program execution. Processes internally track their states which are:
 * 
 *    UNARRIVED: process not yet arrived.
 *    READY: process is in the ready queue.
 *    RUNNING: process is executing its code and making progress towards burst completion.
 *    WAITING: process is doing I/O and will return to the ready queue at a later time.
 *    TERMINATED: process has terminated and finished its context switch out.
 *    SW_IN: process is in a context switch into the CPU.
 *    SW_READY: process has been preempted and in a context switch out of the CPU and into the READY state.
 *    SW_WAIT: process has finished a burst and is being context switched out of the CPU and into the WAITING state.
 *    SW_TERM: process has finished its last burst and is context switching out of the CPU and into the TERMINATED state.
 * 
 *    Process states can be changed by either calling nextState(), which moves the process to the next logical state
 *    if it is not originally in the RUNNING or TERMINATED state, or decrementBurst() which decides the next state of the process
 *    if it is in the RUNNING state. Processes internally track waiting and turnaround times so that the scheduling code isn't responsible
 *    for it.
 *    
 */

#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <string>
#include "globals.hpp"
#include <utility>
#include <functional>
#include <numeric>
#include <cmath>

class Process {

public:
  enum class State: char { UNARRIVED, READY, RUNNING, WAITING, TERMINATED, SW_IN, SW_READY, SW_WAIT, SW_TERM };
  Process(unsigned int arrivalTime, 
    const std::vector<unsigned int>& cpuBurstTimes, 
    const std::vector<unsigned int>& ioBurstTimes,
    unsigned int tau,
    double alpha); 
  std::string nextState(unsigned int timestamp, unsigned int tcs); 
  Process::State getState() const { return processState; }
  Process::State decrementBurst();
  unsigned int getNumBursts() const { return originalCpuBurstTimes.size(); }
  void printInfo() const;
  void preempt(); 
  unsigned int getArrivalTime() const { return arrivalTime; }
  std::pair<unsigned long long, unsigned long long> getTotalCpuBurstTime() const; 
  std::pair<unsigned long long, unsigned long long> getTotalWaitTime() const;
  std::pair<unsigned long long, unsigned long long> getTotalTurnaroundTime() const;
  unsigned int getCurrOriginalCpuBurstTime() const;
  unsigned int getCurrIoBurstTime() const;
  char getPid() const { return pid; }
  unsigned long long getNumPreempts() const { return numPreempts; }
  unsigned long long getNumCtxSwitches() const { return numCtxSwitches; }
  unsigned int getBurstsRemaining() const { return originalCpuBurstTimes.size() - burstIdx - 1; }
  unsigned int getRemainingBurstTime() const;
  unsigned int getTau() const { return tau; }
  signed long long getExpectedRemainingBurstTime() const;
  void reset();
  bool isStartOfBurst() const;
  unsigned int getElapsedBurstTime() const;
private:
  void startWaitingTimer(unsigned int timestamp);
  void endWaitingTimer(unsigned int timestamp);
  void startTurnaroundTimer(unsigned int timestamp);
  void endTurnaroundTimer(unsigned int timestamp); 
  void recalculateTau();
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
  unsigned int numPreempts = 0;
  unsigned int numCtxSwitches = 0;
  const unsigned int tau0;
  unsigned int tau; 
  const double alpha;
  State processState;
};
#endif
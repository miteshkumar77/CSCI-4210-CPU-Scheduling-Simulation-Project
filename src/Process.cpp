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
  waitingTimes(std::vector<unsigned int>(cpuBurstTimes.size(), 0)),
  turnaroundTimes(std::vector<unsigned int>(cpuBurstTimes.size(), 0)),
  processState(State::UNARRIVED) {
  
  if (gpid > 'Z') {
    gpid = 'Z'; 
    throw std::runtime_error("Error: Process Constructor called more than 26 times...");
  }
}

unsigned int Process::getCurrIoBurstTime() const {
  if (burstIdx == cpuBurstTimes.size()) {
    throw std::runtime_error("Error: tried to get burst io time of last cpu burst.");
  }
  return originalIoBurstTimes[burstIdx];
}

void Process::startWaitingTimer(unsigned int timestamp) {
  if (waitingTimer != -1) {
    throw std::runtime_error("Error: startWaitingTimer() called while timer is already running.");
  }
  waitingTimer = timestamp;
}
void Process::endWaitingTimer(unsigned int timestamp) {
  if (waitingTimer == -1) {
    throw std::runtime_error("Error: endWaitingTimer() called while timer is not running.");
  }
  if (burstIdx >= waitingTimes.size()) {
    throw std::runtime_error("Error: endWaitingTimer() called with out of bounds burstIdx.");
  }
  waitingTimes[burstIdx] = waitingTimes[burstIdx] + timestamp - waitingTimer; 
  waitingTimer = -1;
}
void Process::startTurnaroundTimer(unsigned int timestamp) {
  if (turnaroundTimer != -1) {
    throw std::runtime_error("Error: startTurnaroundTimer() called while timer is already running.");
  }
  turnaroundTimer = timestamp + 1;
}
void Process::endTurnaroundTimer(unsigned int timestamp) {
  if (turnaroundTimer == -1) {
    throw std::runtime_error("Error: endTurnaroundTimer() called while timer is not running.");
  }
  if (burstIdx >= turnaroundTimes.size()) {
    throw std::runtime_error("Error: endTurnaroundTimer() called with out of bounds burstIdx.");
  }
  turnaroundTimes[burstIdx] += timestamp - turnaroundTimer; 
  turnaroundTimer = -1;
}
// burstIndex
void Process::nextState(unsigned int timestamp, unsigned int tcs) {
  switch (processState) {
    case Process::State::UNARRIVED: // -> READY
      processState = Process::State::READY;
      startWaitingTimer(timestamp);
      startTurnaroundTimer(timestamp);
      break;
    case Process::State::READY: // -> SW_IN
      processState = Process::State::SW_IN;
      endWaitingTimer(timestamp);
      break;
    case Process::State::SW_READY: // -> READY
      processState = Process::State::READY;
      startWaitingTimer(timestamp);
      break;
    case Process::State::SW_IN: // -> RUNNING
      processState = Process::State::RUNNING; 
      break;
    case Process::State::SW_WAIT: // -> WAITING
      processState = Process::State::WAITING;
      endTurnaroundTimer(timestamp);
      break;
    case Process::State::WAITING: // -> READY
      ++burstIdx;
      processState = Process::State::READY;
      startWaitingTimer(timestamp); 
      startTurnaroundTimer(timestamp);
      break;
    case Process::State::SW_TERM: // -> TERMINATED
      processState = Process::State::TERMINATED;
      endTurnaroundTimer(timestamp);
      ++burstIdx;
      break;
    case Process::State::RUNNING:
      throw std::runtime_error("Error: next state of RUNNING process must be set by preempt() or decrementBurst().");
    case Process::State::TERMINATED:
      throw std::runtime_error("Error: called nextState() on TERMINATED process."); 
    default:
      throw std::runtime_error("Error: nextState() called for unrecognized process state.");
  }
}

Process::State Process::decrementBurst() {
  if (processState != Process::State::RUNNING) {
    throw std::runtime_error("Error: decrementBurst() called for a non-running process.");
  }
  if (burstIdx >= cpuBurstTimes.size()) {
    std::cout << "Burst Idx: " << burstIdx << std::endl; 
    throw std::runtime_error("Error: Tried to decrement with burstIdx out of bounds.");
  }
  if (0 == --cpuBurstTimes[burstIdx]) {
    if (burstIdx + 1 == cpuBurstTimes.size()) {
      processState = Process::State::SW_TERM; 
    } else {
      processState = Process::State::SW_WAIT; 
    }
  }
  return processState;
}

void Process::preempt() {
  if (processState != Process::State::RUNNING) {
    throw std::runtime_error("Error: preempt() called for a non-RUNNING process.");
  }
  processState = Process::State::SW_READY;
}

void Process::printInfo() const {
  std::cout << "pid: " << pid << std::endl;
  std::cout << "arrival time: " << arrivalTime << std::endl;
  std::cout << "current burst index: " << burstIdx << std::endl;

  std::cout << "remaining cpu burst times: ";
  for (auto i : cpuBurstTimes) std::cout << i << ' ';
  std::cout << std::endl;

  std::cout << "original cpu burst times: ";
  for (auto i : originalCpuBurstTimes) std::cout << i << ' ';
  std::cout << std::endl;

  std::cout << "io burst times: ";
  for (auto i : originalIoBurstTimes) std::cout << i << ' ';
  std::cout << std::endl;

  std::cout << "turnaround times: ";
  for (auto i : turnaroundTimes) std::cout << i << ' ';
  std::cout << std::endl;

  std::cout << "wait times: ";
  for (auto i : waitingTimes) std::cout << i << ' ';
  std::cout << std::endl;
  std::cout << std::endl;
}
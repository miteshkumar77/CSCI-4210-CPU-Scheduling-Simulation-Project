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
  ioBurstTimes(std::move(ioBurstTimes)),
  processState(State::UNARRIVED) {
  
  if (gpid > 'Z') {
    gpid = 'Z'; 
    throw std::runtime_error("Error: Process Constructor called more than 26 times...");
  }
}



void Process::reset() {
  burstIdx = 0;
  int n = cpuBurstTimes.size(); 
  for (int i = 0; i < n; ++i) {
    cpuBurstTimes[i] = originalIoBurstTimes[i];
  }

  for (int i = 0; i < n; ++i) {
    ioBurstTimes[i] = ioBurstTimes[i];
  }
}

void Process::nextState() {
  if (processState == State::UNARRIVED || processState == State::WAITING) {
    processState = State::READY; 
  } else if (processState == State::READY) {
    processState = State::RUNNING;
  } else if (processState == State::RUNNING) {
    processState = State::WAITING; 
  } else {
    throw std::runtime_error("Error: nextState() called for terminated process.");
  }
}

void Process::terminate() {
  if (processState != State::RUNNING) {
    throw std::runtime_error("Error: terminate() called for a non-running process.");
  }
  processState = State::TERMINATED; 
}
void Process::preempt() {
  if (processState != State::RUNNING) {
    throw std::runtime_error("Error: preempt() called for a non-running process.");
  }
  processState = State::READY;
}

Process::State Process::decrementBurst() {
  if (processState != State::RUNNING) {
    throw std::runtime_error("Error: Tried to decrement burst for a non-running process.");
  }
  if (burstIdx >= cpuBurstTimes.size()) {
    std::cout << "Burst Idx: " << burstIdx << std::endl;
    throw std::runtime_error("Error: Tried to decrement with burstIdx out of bounds.");
  }
  if (0 == --cpuBurstTimes[burstIdx]) {
    if (++burstIdx == cpuBurstTimes.size()) {
      terminate(); 
    } else {
      nextState(); 
    }
  }
  return processState; 
}

unsigned int Process::getCurrIoBurstTime() const {
  if (processState != State::WAITING) {
    throw std::runtime_error("Error: tried to get burst Io time while process isn't in waiting state.");
  }
  if (burstIdx == 0) {
    throw std::runtime_error("Error: burstIdx was unexpectedly 0 when trying to get IoBurstTime.");
  }

  return ioBurstTimes[burstIdx - 1];
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
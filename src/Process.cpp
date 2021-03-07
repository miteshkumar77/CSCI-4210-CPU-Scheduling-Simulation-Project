#include "Process.hpp"

char Process::gpid = 'A' - 1; 

Process::Process(unsigned int arrivalTime, 
  const std::vector<unsigned int>& cpuBurstTimes, 
  const std::vector<unsigned int>& ioBurstTimes): 
  arrivalTime(arrivalTime), 
  pid(++gpid), 
  originalCpuBurstTimes(std::vector<unsigned int>(cpuBurstTimes.begin(), cpuBurstTimes.end())),
  originalIoBurstTimes(std::vector<unsigned int>(ioBurstTimes.begin(), ioBurstTimes.end())),
  turnaroundTimes(std::vector<unsigned int>(cpuBurstTimes.size())),
  cpuBurstTimes(std::move(cpuBurstTimes)),
  waitTimes(std::vector<unsigned int>(cpuBurstTimes.size())),
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
    cpuBurstTimes[i] = originalCpuBurstTimes[i];
  }
}

void Process::nextState(unsigned int timestamp, unsigned int ctxSwitchDelay) {
  if (processState == State::UNARRIVED || processState == State::WAITING) {
    waitTimes[burstIdx] = 0; // Clear any existing data
    turnaroundTimes[burstIdx] = timestamp; // Start tracking turnaround time
    timeJoinedReadyQueue = timestamp + 1;
    processState = State::READY; 
  } else if (processState == State::READY) {
    waitTimes[burstIdx] += timestamp - timeJoinedReadyQueue;
    processState = State::RUNNING;
  } else if (processState == State::RUNNING) {
    processState = State::WAITING; 
  } else {
    throw std::runtime_error("Error: nextState() called for terminated process.");
  }
}

void Process::makeEntry() {
  switch(processState) {
    case State::RUNNING:
      log.push_back('X');
      break;
    case State::UNARRIVED:
      log.push_back('O');
      break;
    case State::WAITING:
      log.push_back('W');
      break;
    case State::READY:
      log.push_back(' ');
      break;
    case State::TERMINATED:
      log.push_back('T');
      break;
    default:
      log.push_back('?');
      break;
  }
}

void Process::terminate(unsigned int timestamp, unsigned int ctxSwitchDelay) {
  if (processState != State::RUNNING) {
    throw std::runtime_error("Error: terminate() called for a non-running process.");
  }
  processState = State::TERMINATED; 
}
void Process::preempt(unsigned int timestamp, unsigned int ctxSwitchDelay) {
  if (processState != State::RUNNING) {
    throw std::runtime_error("Error: preempt() called for a non-running process.");
  }
  timeJoinedReadyQueue = timestamp + 1;
  processState = State::READY;
}

Process::State Process::decrementBurst(unsigned int timestamp, unsigned int ctxSwitchDelay) {
  if (processState != State::RUNNING) {
    throw std::runtime_error("Error: Tried to decrement burst for a non-running process.");
  }
  if (burstIdx >= cpuBurstTimes.size()) {
    std::cout << "Burst Idx: " << burstIdx << std::endl;
    throw std::runtime_error("Error: Tried to decrement with burstIdx out of bounds.");
  }
  if (0 == --cpuBurstTimes[burstIdx]) {
    turnaroundTimes[burstIdx] = timestamp - turnaroundTimes[burstIdx] + ctxSwitchDelay;
    if (++burstIdx == cpuBurstTimes.size()) {
      terminate(timestamp, ctxSwitchDelay); 
    } else {
      nextState(timestamp, ctxSwitchDelay); 
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

  return originalIoBurstTimes[burstIdx - 1];
}

void Process::printProcess() {
  std::cout << "pid: " << pid << " ";
  for (auto i : log) std::cout << i;
  std::cout << std::endl;
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
  for (auto i : waitTimes) std::cout << i << ' ';
  std::cout << std::endl;
  std::cout << std::endl;
}
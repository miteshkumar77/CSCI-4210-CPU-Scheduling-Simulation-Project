/**
 *  CSCI 4210 Operating Systems
 *  2021 Spring
 *
 *  Simulation Project - Process.cpp
 *
 *  Authors:
 *    Mitesh Kumar  [ kumarm4 ]
 *    Jason Lam     [  lamj7  ]
 *    William He    [  hew7   ]
 */

#include "Process.hpp"

char Process::gpid = 'A' - 1;

Process::Process(unsigned int arrivalTime,
                 const std::vector<unsigned int> &cpuBurstTimes,
                 const std::vector<unsigned int> &ioBurstTimes,
                 unsigned int tau, double alpha)
    : arrivalTime(arrivalTime), pid(++gpid),
      originalCpuBurstTimes(std::vector<unsigned int>(cpuBurstTimes.begin(),
                                                      cpuBurstTimes.end())),
      originalIoBurstTimes(
          std::vector<unsigned int>(ioBurstTimes.begin(), ioBurstTimes.end())),
      cpuBurstTimes(std::move(cpuBurstTimes)),
      waitingTimes(std::vector<unsigned int>(cpuBurstTimes.size(), 0)),
      turnaroundTimes(std::vector<unsigned int>(cpuBurstTimes.size(), 0)),
      tau0(tau), tau(tau), alpha(alpha), processState(State::UNARRIVED) {

  if (gpid > 'Z') {
    gpid = 'Z';
    throw std::runtime_error(
        "Error: Process Constructor called more than 26 times...");
  }
}

void Process::reset() {

  unsigned int n = cpuBurstTimes.size();
  if (burstIdx < n) {
    throw std::runtime_error("Error: Process::reset() called for a process "
                             "that hasn't fully completed.");
  }

  for (int i : cpuBurstTimes) {
    if (i) {
      throw std::runtime_error(
          "Error: found a non-zero cpuBurstTime when Process::reset() called");
    }
  }

  for (unsigned int i = 0; i < n; ++i) {
    cpuBurstTimes[i] = originalCpuBurstTimes[i];
    waitingTimes[i] = 0;
    turnaroundTimes[i] = 0;
    burstIdx = 0;
    numPreempts = 0;
    numCtxSwitches = 0;
  }
  tau = tau0;
  processState = Process::State::UNARRIVED;
}

std::pair<unsigned long long, unsigned long long>
Process::getTotalCpuBurstTime() const {
  return {std::accumulate(originalCpuBurstTimes.begin(),
                          originalCpuBurstTimes.end(), 0),
          originalCpuBurstTimes.size()};
}
std::pair<unsigned long long, unsigned long long>
Process::getTotalWaitTime() const {
  return {std::accumulate(waitingTimes.begin(), waitingTimes.end(), 0),
          waitingTimes.size()};
}

std::pair<unsigned long long, unsigned long long>
Process::getTotalTurnaroundTime() const {
  return {std::accumulate(turnaroundTimes.begin(), turnaroundTimes.end(), 0),
          turnaroundTimes.size()};
}

unsigned int Process::getCurrIoBurstTime() const {
  if (burstIdx >= cpuBurstTimes.size() - 1) {
    throw std::runtime_error(
        "Error: tried to get burst io time of last cpu burst.");
  }
  return originalIoBurstTimes[burstIdx];
}

unsigned int Process::getCurrOriginalCpuBurstTime() const {
  if (burstIdx >= cpuBurstTimes.size()) {
    throw std::runtime_error(
        "Error: tried to get burst io time of last cpu burst.");
  }
  return originalCpuBurstTimes[burstIdx];
}

void Process::recalculateTau() {
  if ((processState != Process::State::SW_WAIT) &&
      (processState != Process::State::SW_TERM)) {
    throw std::runtime_error("Error: called recalculateTau() for process that "
                             "isn't in SW_WAIT or SW_TERM");
  }
  if (burstIdx >= originalCpuBurstTimes.size()) {
    throw std::runtime_error(
        "Error: called recalculateTau() with out of bounds burstIdx.");
  }
  tau = ceil(originalCpuBurstTimes[burstIdx] * alpha + (1 - alpha) * tau);
}

void Process::startWaitingTimer(unsigned int timestamp) {
  if (waitingTimer != -1) {
    throw std::runtime_error(
        "Error: startWaitingTimer() called while timer is already running.");
  }
  waitingTimer = timestamp;
}
void Process::endWaitingTimer(unsigned int timestamp) {
  if (waitingTimer == -1) {
    throw std::runtime_error(
        "Error: endWaitingTimer() called while timer is not running.");
  }
  if (burstIdx >= waitingTimes.size()) {
    throw std::runtime_error(
        "Error: endWaitingTimer() called with out of bounds burstIdx.");
  }
  waitingTimes[burstIdx] = waitingTimes[burstIdx] + timestamp - waitingTimer;
  waitingTimer = -1;
}
void Process::startTurnaroundTimer(unsigned int timestamp) {
  if (turnaroundTimer != -1) {
    throw std::runtime_error(
        "Error: startTurnaroundTimer() called while timer is already running.");
  }
  turnaroundTimer = timestamp;
}
void Process::endTurnaroundTimer(unsigned int timestamp) {
  if (turnaroundTimer == -1) {
    throw std::runtime_error(
        "Error: endTurnaroundTimer() called while timer is not running.");
  }
  if (burstIdx >= turnaroundTimes.size()) {
    throw std::runtime_error(
        "Error: endTurnaroundTimer() called with out of bounds burstIdx.");
  }
  turnaroundTimes[burstIdx] += timestamp - turnaroundTimer;
  turnaroundTimer = -1;
}

bool Process::isStartOfBurst() const {
  if (burstIdx >= originalCpuBurstTimes.size()) {
    throw std::runtime_error("Error: isStartOfBurst() called for a process "
                             "with out of bounds burstIdx.");
  }
  return cpuBurstTimes[burstIdx] == originalCpuBurstTimes[burstIdx];
}

unsigned int Process::getElapsedBurstTime() const {
  if (burstIdx >= originalCpuBurstTimes.size()) {
    throw std::runtime_error("Error: getElapsedBurstTime() called for a "
                             "process with out of bounds burstIdx.");
  }

  if (originalCpuBurstTimes[burstIdx] < cpuBurstTimes[burstIdx]) {
    throw std::runtime_error("Error: original cpu burst time was lower than "
                             "current cpu burst time.");
  }
  return originalCpuBurstTimes[burstIdx] - cpuBurstTimes[burstIdx];
}

signed long long Process::getExpectedRemainingBurstTime() const {
  if (burstIdx >= originalCpuBurstTimes.size()) {
    throw std::runtime_error("Error: getExpectedRemainingBurstTime() called "
                             "for a process with out of bounds burstIdx.");
  }

  return static_cast<signed long int>(getTau()) -
         static_cast<signed long int>(getElapsedBurstTime());
}
std::string Process::nextState(unsigned int timestamp, unsigned int tcs) {

  std::string detail = "";
  switch (processState) {
  case Process::State::UNARRIVED: // -> READY
    if (timestamp < MAX_OUTPUT_TS) {
      detail = "Process " + std::string(1, getPid()) +
               " arrived; placed on ready queue";
    }
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
    ++numCtxSwitches;
    processState = Process::State::RUNNING;
    if (timestamp < MAX_OUTPUT_TS) {
      if (isStartOfBurst()) {
        detail = "Process " + std::string(1, getPid()) +
                 " started using the CPU for " +
                 std::to_string(cpuBurstTimes[burstIdx]) + "ms burst";
      } else {
        detail = "Process " + std::string(1, getPid()) +
                 " started using the CPU with " +
                 std::to_string(cpuBurstTimes[burstIdx]) + "ms burst remaining";
      }
    }
    break;
  case Process::State::SW_WAIT: // -> WAITING
    processState = Process::State::WAITING;
    endTurnaroundTimer(timestamp);
    break;
  case Process::State::WAITING: // -> READY
    ++burstIdx;
    processState = Process::State::READY;
    if (timestamp < MAX_OUTPUT_TS) {
      detail = "Process " + std::string(1, getPid()) +
               " completed I/O; placed on ready queue";
    }
    startWaitingTimer(timestamp);
    startTurnaroundTimer(timestamp);
    break;
  case Process::State::SW_TERM: // -> TERMINATED
    processState = Process::State::TERMINATED;
    endTurnaroundTimer(timestamp);
    ++burstIdx;
    break;
  case Process::State::RUNNING:
    throw std::runtime_error("Error: next state of RUNNING process must be set "
                             "by preempt() or decrementBurst().");
  case Process::State::TERMINATED:
    throw std::runtime_error(
        "Error: called nextState() on TERMINATED process.");
  default:
    throw std::runtime_error(
        "Error: nextState() called for unrecognized process state.");
  }
  return detail;
}

Process::State Process::decrementBurst() {
  if (processState != Process::State::RUNNING) {
    throw std::runtime_error(
        "Error: decrementBurst() called for a non-running process.");
  }
  if (burstIdx >= cpuBurstTimes.size()) {
    std::cout << "Burst Idx: " << burstIdx << std::endl;
    throw std::runtime_error(
        "Error: Tried to decrement with burstIdx out of bounds.");
  }
  if (0 == --cpuBurstTimes[burstIdx]) {
    if (burstIdx + 1 == cpuBurstTimes.size()) {
      processState = Process::State::SW_TERM;
    } else {
      processState = Process::State::SW_WAIT;
      recalculateTau();
    }
  }
  return processState;
}

unsigned int Process::getRemainingBurstTime() const {
  if (burstIdx == cpuBurstTimes.size()) {
    throw std::runtime_error(
        "Error: getRemainingBurstTime() called for an out of bounds burstIdx.");
  }
  return cpuBurstTimes[burstIdx];
}

void Process::preempt() {
  if (processState != Process::State::RUNNING) {
    throw std::runtime_error(
        "Error: preempt() called for a non-RUNNING process.");
  }
  ++numPreempts;
  processState = Process::State::SW_READY;
}

void Process::printInfo() const {
  std::cout << "pid: " << pid << std::endl;
  std::cout << "arrival time: " << arrivalTime << std::endl;
  std::cout << "current burst index: " << burstIdx << std::endl;

  std::cout << "remaining cpu burst times: ";
  for (auto i : cpuBurstTimes)
    std::cout << i << ' ';
  std::cout << std::endl;

  std::cout << "original cpu burst times: ";
  for (auto i : originalCpuBurstTimes)
    std::cout << i << ' ';
  std::cout << std::endl;

  std::cout << "io burst times: ";
  for (auto i : originalIoBurstTimes)
    std::cout << i << ' ';
  std::cout << std::endl;

  std::cout << "turnaround times: ";
  for (auto i : turnaroundTimes)
    std::cout << i << ' ';
  std::cout << std::endl;

  std::cout << "wait times: ";
  for (auto i : waitingTimes)
    std::cout << i << ' ';
  std::cout << std::endl;
  std::cout << std::endl;
}

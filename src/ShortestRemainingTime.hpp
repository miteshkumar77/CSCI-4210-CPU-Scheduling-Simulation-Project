/**
 *  CSCI 4210 Operating Systems
 *  2021 Spring
 *
 *  Simulation Project - ShortestRemainingTime.hpp
 *
 *  Authors:
 *    Mitesh Kumar  [ kumarm4 ]
 *    Jason Lam     [  lamj7  ]
 *    William He    [  hew7   ]
 *
 *  Brief:
 *    Implementation of Shortest Remaining Time (SRT) scheduling algorithm that
 * can be converted to Shortest Job First (SJF) by passing sjf=true to the
 * constructor.
 *
 *    run() runs the simulation and outputs important events to terminal in
 * chronological order
 *
 *    printInfo() can be called successfully after run() has
 * completed and will print the final calculated statistics of the run to a
 * output stream.
 *
 *    reset() resets the object for a future run and resets the
 * internal processes to their initial state.
 */

#ifndef SHORTESTREMAININGTIME_HPP
#define SHORTESTREMAININGTIME_HPP

#include "Process.hpp"
#include <algorithm>
#include <deque>
#include <exception>
#include <functional>
#include <iostream>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class ShortestRemainingTime {
public:
  ShortestRemainingTime(std::vector<Process> &processes, unsigned int tcs,
                        bool sjf);
  void printInfo(std::ostream &os) const;
  void run();
  void reset();
  void printCsv(std::ostream &os) const;

private:
  typedef std::vector<Process>::iterator ProcessPtr;
  typedef std::pair<unsigned int, ProcessPtr> ioQueueElem;
  void printEvent(const std::string &detail, bool term) const;
  bool isReadyQueueEmpty() const { return readyQueue.empty(); }
  ProcessPtr peekFirstReady() const;
  void popFirstReady();
  void pushReady(ProcessPtr processPtr);
  Process::State decrementBurstTimer();
  void pushIo(ProcessPtr processPtr);

  double calcAvgWaitTime() const;
  double calcAvgTurnaroundTime() const;
  double calcAvgCpuBurstTime() const;
  unsigned long long calcTotalNumCtxSwitches() const;
  unsigned long long calcTotalNumPreemptions() const;
  double calcCpuUtilization() const {
    return 100.0 * (double)cpuUsageTime / timestamp;
  }
  void resetTcsRemaining();
  void checkRep() const;
  void decrementTcs();
  void preemptRunningProc();
  inline bool burstTimerElapsed() const { return burstRemaining == 0; }
  std::string fmtProc(const ProcessPtr &p) const;
  std::string fmtRecalcTau(const ShortestRemainingTime::ProcessPtr &p) const;

  // Independent
  static const std::function<bool(const ProcessPtr &, const ProcessPtr &)>
      processArrivalComparator;
  static const std::function<bool(const ioQueueElem &, const ioQueueElem &)>
      processIoComparator;
  static const std::function<bool(const ProcessPtr &, const ProcessPtr &)>
      readyQueueComparator;
  std::deque<ProcessPtr> readyQueue;
  unsigned int latestProcessIdx = 0;
  unsigned int timestamp = 0;
  unsigned int burstRemaining = 0;
  unsigned int tcsRemaining = 0;

  unsigned int cpuUsageTime = 0;

  // Default
  std::priority_queue<ioQueueElem, std::vector<ioQueueElem>,
                      decltype(processIoComparator)>
      ioQueue;
  const unsigned int tcs;
  const unsigned int numProcs;
  ProcessPtr runningProc;
  ProcessPtr switchingOutProc;
  ProcessPtr switchingInProc;
  const ProcessPtr nullProc;
  const bool sjf;

  // Non-Default
  std::vector<ProcessPtr> orderedProcesses;
};

#endif

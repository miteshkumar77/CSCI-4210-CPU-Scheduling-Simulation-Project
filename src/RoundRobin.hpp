#ifndef ROUNDROBIN_HPP
#define ROUNDROBIN_HPP

#include <deque>
#include <queue>
#include <algorithm>
#include <vector>
#include <functional>
#include <exception>
#include <stdexcept>
#include "Process.hpp"
#include <utility>
class RoundRobin {

private:
  typedef std::vector<Process>::iterator ProcessPtr; 
  typedef std::pair<unsigned int, ProcessPtr> ioQueueElem; 
  RoundRobin(std::vector<Process>& processes, unsigned int tslice, unsigned int ctxSwitchDelay); 
  ProcessPtr peekLastReady() const;
  ProcessPtr peekFirstReady() const;
  void popFirstReady(); 
  void pushLastReady(ProcessPtr processPtr) { readyQueue.push_back(processPtr); } 
  void pushFirstReady(ProcessPtr processPtr) { readyQueue.push_front(processPtr); }
  bool isReadyQueueEmpty() const;
  bool tick(); 
  void preempt(bool front); 
  static const std::function<bool(const ProcessPtr&, const ProcessPtr&)> processArrivalComparator; 
  static const std::function<bool(const ioQueueElem&, const ioQueueElem&)> processIoComparator;
  std::vector<ProcessPtr> orderedProcesses;
  std::deque<ProcessPtr> readyQueue; 
  std::priority_queue<ioQueueElem, std::vector<ioQueueElem>, decltype(processIoComparator)> ioQueue;
  int latestProcessIdx = 0;
  int timestamp = 0;
  const unsigned int tslice;
  const unsigned int ctxSwitchDelay;
  unsigned int burstRemaining;
  unsigned int ctxSwitchRemaining;
  bool addToEnd = true;
  unsigned int numProcs;
};

#endif
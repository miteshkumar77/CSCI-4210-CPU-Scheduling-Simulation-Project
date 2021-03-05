#include "RoundRobin.hpp"

const std::function<bool(const RoundRobin::ProcessPtr&, const RoundRobin::ProcessPtr&)> 
RoundRobin::processArrivalComparator =
[] (const RoundRobin::ProcessPtr& a, const RoundRobin::ProcessPtr& b) -> bool {
  return a -> getArrivalTime() < b -> getArrivalTime() ||
    (a -> getArrivalTime() == b -> getArrivalTime() && a -> getPid() < b -> getPid()); 
};

const std::function<bool(const RoundRobin::ioQueueElem&, const RoundRobin::ioQueueElem&)>
RoundRobin::processIoComparator = 
[] (const RoundRobin::ioQueueElem& a, const RoundRobin::ioQueueElem& b) -> bool {
  return a.first < b.first || 
    (a.first == b.first && a.second -> getPid() < b.second -> getPid()); 
};

RoundRobin::RoundRobin(std::vector<Process>& processes, 
  unsigned int tslice, 
  unsigned int ctxSwitchDelay): 
  ioQueue(processIoComparator),
  tslice(tslice), ctxSwitchDelay(ctxSwitchDelay), numProcs(processes.size()) {
  orderedProcesses.reserve(processes.size()); 
  for (auto it = processes.begin(); it != processes.end(); ++it)
    orderedProcesses.push_back(it);
  
  sort(orderedProcesses.begin(), orderedProcesses.end(), processArrivalComparator);
}

bool RoundRobin::isReadyQueueEmpty() const {
  return readyQueue.empty(); 
}

RoundRobin::ProcessPtr RoundRobin::peekLastReady() const {
  if (isReadyQueueEmpty()) {
    throw std::runtime_error("Error: Attempted to peek last from an empty ready queue."); 
  }
  return readyQueue.back(); 
}

RoundRobin::ProcessPtr RoundRobin::peekFirstReady() const {
  if (isReadyQueueEmpty()) {
    throw std::runtime_error("Error: Attempted to peek first from an empty ready queue."); 
  }
  return readyQueue.front(); 
}


void RoundRobin::popFirstReady() {
  if (isReadyQueueEmpty()) {
    throw std::runtime_error("Error: Attempted to pop first from an empty ready queue.");
  }
  readyQueue.pop_front(); 
}


bool RoundRobin::tick() {
  


  while(!ioQueue.empty() && ioQueue.top().first <= timestamp) {
    if (addToEnd) {
      pushLastReady(ioQueue.top().second);
    } else {
      pushFirstReady(ioQueue.top().second); 
    }
    ioQueue.pop(); 
  }

  while(latestProcessIdx < numProcs && 
    orderedProcesses[latestProcessIdx] -> getArrivalTime() <= timestamp) {
    if (addToEnd) {
      pushLastReady(orderedProcesses[latestProcessIdx]); 
    } else {
      pushFirstReady(orderedProcesses[latestProcessIdx]); 
    }
  }

  ++timestamp;
  return true;
}
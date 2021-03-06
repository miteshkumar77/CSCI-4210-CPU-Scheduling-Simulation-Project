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
  tslice(tslice), ctxSwitchDelay(ctxSwitchDelay),
  burstRemaining(0),
  ctxSwitchRemaining(0),
  numProcs(processes.size()),
  runningProc(processes.end()),
  nullProc(processes.end()) {

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
  
  if (ctxSwitchRemaining) {
    --ctxSwitchRemaining;
  } else {
    if (runningProc == nullProc) {
      if (!isReadyQueueEmpty()) {
        runningProc = peekFirstReady();
        popFirstReady();
        resetCtxSwitchDelay();
      } else if (latestProcessIdx == numProcs && ioQueue.empty()) {
        return false; // Simulation is over.
      }
    } else {
      if (runningProc -> getState() == Process::State::READY) {
        resetBurstTimer();
        runningProc -> nextState();
      }

      if (runningProc -> getState() != Process::State::RUNNING) {
        std::cout << static_cast<int>(runningProc -> getState()) << std::endl;
        throw std::runtime_error("Error: Process on CPU is not in RUNNING state.");
      }
      --burstRemaining;
      Process::State currState = runningProc -> decrementBurst();
      if (currState == Process::State::RUNNING) {
        if (burstRemaining == 0) {
          if (!isReadyQueueEmpty()) {  
            runningProc -> preempt();
            resetCtxSwitchDelay();
            pushLastReady(runningProc);
            runningProc = nullProc;
          } else {
            resetBurstTimer();
          }
        }
      } else if (currState == Process::State::WAITING) {
        resetCtxSwitchDelay();
        ioQueue.push({runningProc -> getCurrIoBurstTime() + timestamp, runningProc});
        runningProc = nullProc;
      } else if (currState == Process::State::TERMINATED) {
        resetCtxSwitchDelay();
        runningProc = nullProc;
      }
    }
  }
  
  while(!ioQueue.empty() && ioQueue.top().first <= timestamp) {
    if (addToEnd) {
      pushLastReady(ioQueue.top().second);
    } else {
      pushFirstReady(ioQueue.top().second); 
    }
    ioQueue.top().second -> nextState(); 
    ioQueue.pop(); 
  }

  while(latestProcessIdx < numProcs && 
    orderedProcesses[latestProcessIdx] -> getArrivalTime() <= timestamp) {
    orderedProcesses[latestProcessIdx] -> nextState(); 
    if (addToEnd) {
      pushLastReady(orderedProcesses[latestProcessIdx]); 
    } else {
      pushFirstReady(orderedProcesses[latestProcessIdx]); 
    }
    ++latestProcessIdx;
  }
  ++timestamp;
  return true; // Simulation continues
}

void RoundRobin::resetBurstTimer() {
  if (ctxSwitchRemaining) {
    throw std::runtime_error("Error: resetBurstTimer() called when ctxSwitchRemaining was not 0.");
  }
  burstRemaining = tslice;
}

void RoundRobin::resetCtxSwitchDelay() { 

  if (ctxSwitchRemaining) {
    throw std::runtime_error("Error: resetCtxSwitchDelay() called when ctxSwitchRemaining was not 0.");
  }
  ctxSwitchRemaining = ctxSwitchDelay/2; 

}
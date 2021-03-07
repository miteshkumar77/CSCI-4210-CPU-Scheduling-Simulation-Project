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
  switchingOutProc(processes.end()),
  switchingInProc(processes.end()),
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

Process::State RoundRobin::decrementBurstTimer() {
  if (burstRemaining == 0) {
    throw std::runtime_error("Error: decrementBurstTimer() was called on a fully elapsed burst timer.");
  }
  if (runningProc == nullProc) {
    throw std::runtime_error("Error: decrementBurstTimer() was called without a process in the CPU.");
  }
  if (runningProc -> getState() != Process::State::RUNNING) {
    throw std::runtime_error("Error: decrementBurstTimer() was called without a process in RUNNING state.");
  }
  
  --burstRemaining;
  return runningProc -> decrementBurst(timestamp, ctxSwitchDelay);
}
void RoundRobin::decrementCtxSwitchTimer() {
  if (ctxSwitchRemaining == 0) {
    throw std::runtime_error("Error: decrementCtxSwitchTimer() was called on a fully elapsed ctx switch timer.");
  }
  --ctxSwitchRemaining;
}

void RoundRobin::preemptRunningProc() {
  if (isReadyQueueEmpty()) {
    throw std::runtime_error("Error: preemptRunningProc() called when ready queue was empty.");
  }
  if (runningProc == nullProc) {
    throw std::runtime_error("Error: preemptRunningProc() called when there was no runningProc.");
  }
  if (runningProc -> getState() != Process::State::RUNNING) {
    throw std::runtime_error("Error: preemptRunningProc() called for a non-running process.");
  }
  runningProc -> preempt(timestamp, ctxSwitchDelay);
  ++numPreempts;
}

bool RoundRobin::tick() {

  if (ctxSwitchRemaining) {
    decrementCtxSwitchTimer();
  } else {
    
    if (switchingOutProc != nullProc && switchingInProc != nullProc) {
      throw std::runtime_error("Error: Process switching in, and switching out simultaneously.");
    }

    if (switchingInProc != nullProc) {
      runningProc = switchingInProc;
      switchingInProc = nullProc;
    } else if (switchingOutProc != nullProc) {
      if (switchingOutProc -> getState() == Process::State::WAITING) {
        ioQueue.push({runningProc -> getCurrIoBurstTime() + timestamp, runningProc});
      } else if (switchingOutProc -> getState() == Process::State::READY) {
        pushLastReady(switchingOutProc); 
      }
      switchingOutProc = nullProc;
    }

    if (runningProc == nullProc) {
      if (!isReadyQueueEmpty()) {
        switchingInProc = peekFirstReady();
        popFirstReady();
        resetCtxSwitchDelay();
        if (!ctxSwitchRemaining) {
          return tick(); 
        }
        --ctxSwitchRemaining; 
      } else if (latestProcessIdx == numProcs && ioQueue.empty() 
              && switchingOutProc == nullProc && switchingInProc == nullProc) {
        
        return false; // Simulation is over.
      }
    } else {
      if (runningProc -> getState() == Process::State::READY) {
        resetBurstTimer();
        runningProc -> nextState(timestamp, ctxSwitchDelay);
      }

      if (runningProc -> getState() != Process::State::RUNNING) {
        std::cout << static_cast<int>(runningProc -> getState()) << std::endl;
        throw std::runtime_error("Error: Process on CPU is not in RUNNING state.");
      }
      Process::State currState = decrementBurstTimer();
      if (currState == Process::State::RUNNING) {
        if (burstTimerElapsed()) {
          if (!isReadyQueueEmpty()) {  
            preemptRunningProc();
            resetCtxSwitchDelay();
            switchingOutProc = runningProc;
            // pushLastReady(runningProc);
            runningProc = nullProc;
          } else {
            resetBurstTimer();
          }
        }
      } else if (currState == Process::State::WAITING) {
        resetCtxSwitchDelay();
        switchingOutProc = runningProc;
        // ioQueue.push({runningProc -> getCurrIoBurstTime() + timestamp, runningProc});
        runningProc = nullProc;
      } else if (currState == Process::State::TERMINATED) {
        resetCtxSwitchDelay();
        switchingOutProc = runningProc;
        runningProc = nullProc;
      }
    }
  }
  
  while(!ioQueue.empty() && ioQueue.top().first <= timestamp) {
    ioQueue.top().second -> nextState(timestamp, ctxSwitchDelay); 
    if (addToEnd) {
      pushLastReady(ioQueue.top().second);
    } else {
      pushFirstReady(ioQueue.top().second); 
    }
    ioQueue.pop(); 
  }

  while(latestProcessIdx < numProcs && 
    orderedProcesses[latestProcessIdx] -> getArrivalTime() <= timestamp) {
    orderedProcesses[latestProcessIdx] -> nextState(timestamp, ctxSwitchDelay); 
    if (addToEnd) {
      pushLastReady(orderedProcesses[latestProcessIdx]); 
    } else {
      pushFirstReady(orderedProcesses[latestProcessIdx]); 
    }
    ++latestProcessIdx;
  }

  
  for (ProcessPtr p: orderedProcesses) {
    p -> makeEntry();
  }
  std::cout << timestamp << "| Q: ";
  for (auto& p : readyQueue) {
    std::cout << p -> getPid() << ": " << p -> getCurrCpuBurstTime() << ", ";
  }
  if (switchingInProc != nullProc) {
    std::cout << " Context Switching In: " << switchingInProc -> getPid() << ": " << switchingInProc -> getCurrCpuBurstTime();
  } else if (runningProc != nullProc) {
    std::cout << " Running: " << runningProc -> getPid() << ": " << runningProc -> getCurrCpuBurstTime(); 
  }
  std::cout << std::endl;
  ++timestamp;
  return true; // Simulation continues
}

void RoundRobin::pushLastReady(RoundRobin::ProcessPtr processPtr) { 
  if (processPtr -> getState() != Process::State::READY) {
    throw std::runtime_error("Error: pushLastReady() called for process that wasn't in the READY state."); 
  }
  readyQueue.push_back(processPtr); 
}   

void RoundRobin::pushFirstReady(RoundRobin::ProcessPtr processPtr) {
  if (processPtr -> getState() != Process::State::READY) {
    throw std::runtime_error("Error: pushFirstReady() called for process that wasn't in the READY state."); 
  }
  readyQueue.push_front(processPtr); 
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
  ++numCtxSwitches; 
  ctxSwitchRemaining = ctxSwitchDelay/2; 

}

void RoundRobin::printInfo() const {
  std::cout << "Number of preempts: " << numPreempts << std::endl;
  std::cout << "Number of context switches: " << numCtxSwitches << std::endl;
  std::cout << "Total execution time: " << timestamp << "ms" << std::endl;
}
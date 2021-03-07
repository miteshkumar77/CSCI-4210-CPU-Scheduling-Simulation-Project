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

RoundRobin::RoundRobin(std::vector<Process>& procs,
  unsigned int tslice,
  unsigned int tcs):
  ioQueue(processIoComparator),
  tslice(tslice), tcs(tcs), numProcs(procs.size()),
  runningProc(procs.end()),
  switchingOutProc(procs.end()),
  switchingInProc(procs.end()),
  nullProc(procs.end()) {
    orderedProcesses.reserve(procs.size()); 
    for (auto it = procs.begin(); it != procs.end(); ++it)
      orderedProcesses.push_back(it); 
    
    sort(orderedProcesses.begin(), orderedProcesses.end(), processArrivalComparator);
}

void RoundRobin::decrementTcs() {
  if (switchingOutProc == nullProc && switchingInProc == nullProc) {
    throw std::runtime_error("Error: decrementTcs() when there isn't a process in context switch.");
  }
  if (!tcsRemaining) {
    throw std::runtime_error("Error: decrementTcs() called when tcsRemaining is 0.");
  }
  --tcsRemaining;
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
  return runningProc -> decrementBurst(); 
}

void RoundRobin::resetTcsRemaining() {
  if (tcsRemaining) {
    throw std::runtime_error("Error: resetTcsRemaining() called when tcsRemaining was not 0.");
  }
  ++numCtxSwitches;
  tcsRemaining = tcs/2;
}

void RoundRobin::resetBurstTimer() {
  // resetBurstTimer when burstRemaining is non-zero is OK, unlike for tcsRemaining
  if (tcsRemaining) {
    throw std::runtime_error("Error: resetBurstTimer() called when ctxSwitchRemaining was not 0.");
  }
  burstRemaining = tslice;
}

void RoundRobin::preemptRunningProc() {
  if (isReadyQueueEmpty()) {
    throw std::runtime_error("Error: preemptRunningProc() called when ready queue was empty.");
  }
  if (runningProc == nullProc) {
    throw std::runtime_error("Error: preemptRunningProc() called when there was no runningProc.");
  }
  if (runningProc -> getState() != Process::State::RUNNING) {
    throw std::runtime_error("Error: preemptRunningProc() called for a non-RUNNING process.");
  }
  runningProc -> preempt();
  ++numPreempts;
}

void RoundRobin::pushIo(ProcessPtr processPtr) {
  if (processPtr -> getState() != Process::State::WAITING) {
    throw std::runtime_error("Error: pushIo() called for process that isn't in WAITING state.");
  }
  ioQueue.push({processPtr -> getCurrIoBurstTime() + timestamp, processPtr});
}

bool RoundRobin::tick() {
  if (tcsRemaining) {
    decrementTcs(); 
  } else {
    if ((switchingOutProc != nullProc || switchingInProc != nullProc) &&
          runningProc != nullProc) {
        
      throw std::runtime_error("Error: Process running while there is a process in context switch.");
    }
    if (switchingOutProc != nullProc && switchingInProc != nullProc) {
      throw std::runtime_error("Error: Processes switching in, and switching out simultaneously.");
    }

    if (switchingInProc != nullProc) {
      if (switchingInProc -> getState() != Process::State::SW_IN) {
        throw std::runtime_error("Error: Switching in process did not have correct SW_IN process state.");
      }
      switchingInProc -> nextState(timestamp, tcs); 
      resetBurstTimer();
      if (switchingInProc -> getState() != Process::State::RUNNING) {
        throw std::runtime_error("Error: SW_IN process did not switch to RUNNING state.");
      }
      runningProc = switchingInProc;
      switchingInProc = nullProc;
    } else if (switchingOutProc != nullProc) {
      if (switchingOutProc -> getState() == Process::State::SW_WAIT) {
        switchingOutProc -> nextState(timestamp, tcs); 
        pushIo(switchingOutProc);
        switchingOutProc = nullProc;
      } else if (switchingOutProc -> getState() == Process::State::SW_READY) {
        switchingOutProc -> nextState(timestamp, tcs);
        pushLastReady(switchingOutProc);
        switchingOutProc = nullProc;
      } else if (switchingOutProc -> getState() == Process::State::SW_TERM) {
        switchingOutProc -> nextState(timestamp, tcs);
        if (switchingOutProc -> getState() != Process::State::TERMINATED) {
          throw std::runtime_error("Error: SW_TERM process did not switch to TERMINATED state.");
        }
        switchingOutProc = nullProc;
      } else {
        throw std::runtime_error("Error: switching out process was not in SW_WAIT, SW_READY, or SW_TERM state.");
      }
    }

    if (runningProc == nullProc) {
      if (!isReadyQueueEmpty()) {
        switchingInProc = peekFirstReady();
        if(switchingInProc -> getState() != Process::State::READY) {
          throw std::runtime_error("Error: process that was pulled from ready queue was not in READY state.");
        }
        popFirstReady();
        switchingInProc -> nextState(timestamp, tcs);
        if (switchingInProc -> getState() != Process::State::SW_IN) {
          throw std::runtime_error("Error: READY process did not switch to SW_IN state.");
        }
        resetTcsRemaining();
        if (!tcsRemaining) {
          return tick(); 
        }
        --tcsRemaining;
      } else if (latestProcessIdx == numProcs && ioQueue.empty()
        && switchingInProc == nullProc && switchingOutProc == nullProc) {

        return false; // Simulation is over.
      }
    } else {
      if (runningProc -> getState() != Process::State::RUNNING) {
        throw std::runtime_error("Error: SW_IN process did not switch to RUNNING state.");
      }

      Process::State currState = decrementBurstTimer(); 
      if (currState == Process::State::RUNNING) {
        if (burstTimerElapsed()) {
          if (!isReadyQueueEmpty()) {
            preemptRunningProc(); 
            resetTcsRemaining();
            switchingOutProc = runningProc;
            runningProc = nullProc;
          } else {
            resetBurstTimer(); 
          }
        }
      } else if (currState == Process::State::SW_WAIT) {
        resetTcsRemaining();
        switchingOutProc = runningProc;
        runningProc = nullProc;
      } else if (currState == Process::State::SW_TERM) {
        resetTcsRemaining();
        switchingOutProc = runningProc;
        runningProc = nullProc;
      } else {
        throw std::runtime_error("Error: runningProc wasn't in RUNNING, SW_WAIT, or SW_TERM stage after decrement.");
      }
    }
  }

  while(!ioQueue.empty() && ioQueue.top().first <= timestamp) {
    if (ioQueue.top().second -> getState() != Process::State::WAITING) {
      throw std::runtime_error("Error: process in ioQueue was not in WAITING state.");
    }
    ioQueue.top().second -> nextState(timestamp, tcs); 
    if (ioQueue.top().second -> getState() != Process::State::READY) {
      throw std::runtime_error("Error: WAITING process did not switch to READY state.");
    }
    if (addToEnd) {
      pushLastReady(ioQueue.top().second);
    } else {
      pushFirstReady(ioQueue.top().second);
    }
    ioQueue.pop();
  }

  while(latestProcessIdx < numProcs &&
    orderedProcesses[latestProcessIdx] -> getArrivalTime() <= timestamp) {
    if (orderedProcesses[latestProcessIdx] -> getState() != Process::State::UNARRIVED) {
      throw std::runtime_error("Error: unarrived process was not in UNARRIVED state.");
    }
    orderedProcesses[latestProcessIdx] -> nextState(timestamp, tcs);
    if (orderedProcesses[latestProcessIdx] -> getState() != Process::State::READY) {
      throw std::runtime_error("Error: UNARRIVED process did not switch to READY state.");
    }
    if (addToEnd) {
      pushLastReady(orderedProcesses[latestProcessIdx]);
    } else {
      pushFirstReady(orderedProcesses[latestProcessIdx]);
    }
    ++latestProcessIdx;
  }
  ++timestamp;
  return true;
}


void RoundRobin::printInfo() const {
  std::cout << "Number of preempts: " << numPreempts << std::endl;
  std::cout << "Number of context switches: " << numCtxSwitches << std::endl;
  std::cout << "Total execution time: " << timestamp - 1 << "ms" << std::endl;
}
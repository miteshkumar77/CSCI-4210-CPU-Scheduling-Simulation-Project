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
  return a.first > b.first || 
    (a.first == b.first && a.second -> getPid() > b.second -> getPid()); 
};

RoundRobin::RoundRobin(std::vector<Process>& procs,
  unsigned int tslice,
  unsigned int tcs,
  bool addToEnd,
  bool fcfs):
  ioQueue(processIoComparator),
  tslice(tslice), tcs(tcs), numProcs(procs.size()),
  runningProc(procs.end()),
  switchingOutProc(procs.end()),
  switchingInProc(procs.end()),
  nullProc(procs.end()),
  addToEnd(addToEnd),
  fcfs(fcfs) {
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
  if (fcfs) {
    throw std::runtime_error("Error: preemptRunningProc() calle for FCFS.");
  }
  printEvent("Time slice expired; process " + std::string(1, runningProc -> getPid()) + " preempted with " + std::to_string(runningProc -> getRemainingBurstTime()) + "ms to go", false);
  runningProc -> preempt();
}

void RoundRobin::pushIo(ProcessPtr processPtr) {
  if (processPtr -> getState() != Process::State::WAITING) {
    throw std::runtime_error("Error: pushIo() called for process that isn't in WAITING state.");
  }
  ioQueue.push({processPtr -> getCurrIoBurstTime() + timestamp, processPtr});
}

void RoundRobin::printEvent(const std::string& detail, bool term) const {
  if (!term && timestamp > MAX_OUTPUT_TS) return;
  if (detail == "") return;
  std::cout << "time " << timestamp << "ms: " << detail << " [Q ";
  for (auto it = readyQueue.begin(); !readyQueue.empty() && it != prev(readyQueue.end()); ++it) {
    std::cout << (*it) -> getPid() << " ";
  }
  std::cout << (readyQueue.empty()?"<empty>":std::string(1, readyQueue.back() -> getPid())) << "]" << std::endl;
}


/*


A if context switch in completed, start running the process
B decrement burst timer on running process, preempt or finish burst and start a context switch out
C if no running process or context switching process, pull next process from ready queue and begin context switch in
D io queue to ready queue
E new process arrival add to ready queue


*/


void RoundRobin::run() {
  for (char it = 'A'; it <= 'A' + orderedProcesses.size() - 1; ++it) {
    for (auto& proc : orderedProcesses) {
      if (proc -> getPid() == it) {
        std::cout << "Process " << it << " [NEW] (arrival time " << proc -> getArrivalTime() << " ms) ";
        std::cout << proc -> getNumBursts() << " CPU burst" << (proc -> getNumBursts() == 1?"":"s") << std::endl;
      }
    }
  }
  printEvent("Simulator started for " + std::string((fcfs?"FCFS":"RR with time slice " + std::to_string(tslice) + 
    "ms and rr_add to " + (addToEnd?"END":"BEGINNING"))) , false);
  std::string detail;
  while(true) { // <<< BEGIN RR/FCFS
    if (switchingInProc == nullProc && 
        switchingOutProc == nullProc && 
        runningProc == nullProc && 
        isReadyQueueEmpty() && 
        latestProcessIdx >= orderedProcesses.size() &&
        ioQueue.empty()) {

      break;
    }
    

    if (tcsRemaining) {
      decrementTcs(); 
    }

    if ((switchingOutProc != nullProc || switchingInProc != nullProc) &&
          runningProc != nullProc) {
        
      throw std::runtime_error("Error: Process running while there is a process in context switch.");
    }
    if (switchingOutProc != nullProc && switchingInProc != nullProc) {
      throw std::runtime_error("Error: Processes switching in, and switching out simultaneously.");
    }

    if (!tcsRemaining && switchingOutProc != nullProc) {
      if (switchingOutProc -> getState() == Process::State::SW_WAIT) {
        printEvent(switchingOutProc -> nextState(timestamp, tcs), false); 
        
        pushIo(switchingOutProc);
        switchingOutProc = nullProc;
      } else if (switchingOutProc -> getState() == Process::State::SW_READY) {
        printEvent(switchingOutProc -> nextState(timestamp, tcs), false);
        pushLastReady(switchingOutProc);
        switchingOutProc = nullProc;
      } else if (switchingOutProc -> getState() == Process::State::SW_TERM) {
        printEvent(switchingOutProc -> nextState(timestamp, tcs), true);
        if (switchingOutProc -> getState() != Process::State::TERMINATED) {
          throw std::runtime_error("Error: SW_TERM process did not switch to TERMINATED state.");
        }
        switchingOutProc = nullProc;
      } else {
        throw std::runtime_error("Error: switching out process was not in SW_WAIT, SW_READY, or SW_TERM state.");
      }
    }

    // (a) CPU burst completion
    if (runningProc != nullProc) {
      if (runningProc -> getState() != Process::State::RUNNING) {
        throw std::runtime_error("Error: SW_IN process did not switch to RUNNING state");
      }

      Process::State currState = decrementBurstTimer();
      if (currState == Process::State::RUNNING) {
        if (burstTimerElapsed()) {
          if (!isReadyQueueEmpty() && !fcfs) {
            preemptRunningProc();
            resetTcsRemaining();
            switchingOutProc = runningProc;

            runningProc = nullProc;
          } else {
            if (!fcfs) {
              printEvent("Time slice expired; no preemption because ready queue is empty", false);
            }
            resetBurstTimer();
          }
        }
      } else if (currState == Process::State::SW_WAIT) {
        printEvent("Process " + std::string(1, runningProc -> getPid()) + " completed a CPU burst; " +
          std::to_string(runningProc -> getBurstsRemaining()) + " burst" + (runningProc -> getBurstsRemaining() == 1?" ":"s ") + "to go", false);
        resetTcsRemaining(); 
        switchingOutProc = runningProc;
        runningProc = nullProc;
        printEvent("Process " + std::string(1, switchingOutProc -> getPid()) + 
          " switching out of CPU; will block on I/O until time " + std::to_string(switchingOutProc -> getCurrIoBurstTime() + timestamp + tcsRemaining) + "ms", false);
      } else if (currState == Process::State::SW_TERM) {
        printEvent("Process " + std::string(1, runningProc -> getPid()) + " terminated", true);
        resetTcsRemaining(); 
        switchingOutProc = runningProc;
        runningProc = nullProc;
      } else {
        throw std::runtime_error("Error: runningProc wasn't in RUNNING, SW_WAIT, or SW_TERM stage after decrementBurst");
      }
    }


    if (!tcsRemaining && switchingInProc != nullProc) {
      if (switchingInProc -> getState() != Process::State::SW_IN) {
        throw std::runtime_error("Error: Switching in process did not have correct SW_IN process state.");
      }
      printEvent(switchingInProc -> nextState(timestamp, tcs), false); 
      
      resetBurstTimer();
      if (switchingInProc -> getState() != Process::State::RUNNING) {
        throw std::runtime_error("Error: SW_IN process did not switch to RUNNING state.");
      }
      runningProc = switchingInProc;
      switchingInProc = nullProc;
    }

    // (b) I/O burst completions
    while(!ioQueue.empty() && ioQueue.top().first <= timestamp) {
      if (ioQueue.top().second -> getState() != Process::State::WAITING) {
        throw std::runtime_error("Error: process in ioQueue was not in WAITING state.");
      }
      detail = ioQueue.top().second -> nextState(timestamp, tcs); 
      if (ioQueue.top().second -> getState() != Process::State::READY) {
        throw std::runtime_error("Error: WAITING process did not switch to READY state.");
      }
      if (addToEnd) {
        pushLastReady(ioQueue.top().second);
      } else {
        pushFirstReady(ioQueue.top().second);
      }
      printEvent(detail, false);
      ioQueue.pop(); 
    }
    
    // (c) new process arrivals
    while(latestProcessIdx < numProcs &&
      orderedProcesses[latestProcessIdx] -> getArrivalTime() <= timestamp) {
      if (orderedProcesses[latestProcessIdx] -> getState() != Process::State::UNARRIVED) {
        throw std::runtime_error("Error: unarrived process was not in UNARRIVED state.");
      }
      detail = orderedProcesses[latestProcessIdx] -> nextState(timestamp, tcs);
      if (orderedProcesses[latestProcessIdx] -> getState() != Process::State::READY) {
        throw std::runtime_error("Error: UNARRIVED process did not switch to READY state.");
      }
      if (addToEnd) {
        pushLastReady(orderedProcesses[latestProcessIdx]);
      } else {
        pushFirstReady(orderedProcesses[latestProcessIdx]);
      }
      printEvent(detail, false);
      ++latestProcessIdx;
    }
    
    if (!tcsRemaining && runningProc == nullProc && switchingInProc == nullProc && switchingOutProc == nullProc && !isReadyQueueEmpty()) {
      switchingInProc = peekFirstReady();
      if(switchingInProc -> getState() != Process::State::READY) {
        throw std::runtime_error("Error: process that was pulled from ready queue was not in READY state.");
      }
      popFirstReady();
      printEvent(switchingInProc -> nextState(timestamp, tcs), false);
      if (switchingInProc -> getState() != Process::State::SW_IN) {
        throw std::runtime_error("Error: READY process did not switch to SW_IN state.");
      }
      resetTcsRemaining();
      
    }
  
    ++timestamp;
  } // <<< END RR/FCFS
  --timestamp;
  printEvent("Simulator ended for " + std::string((fcfs?"FCFS":"RR")), true); 

}

bool RoundRobin::tick() {
  if (timestamp == 0) {
    for (char it = 'A'; it <= 'A' + orderedProcesses.size() - 1; ++it) {
      for (auto& proc : orderedProcesses) {
        if (proc -> getPid() == it) {
          std::cout << "Process " << it << " [NEW] (arrival time " << proc -> getArrivalTime() << " ms) ";
          std::cout << proc -> getNumBursts() << " CPU burst" << (proc -> getNumBursts() == 1?"":"s") << std::endl;
        }
      }
    }
    printEvent("Simulator started for " + std::string((fcfs?"FCFS":"RR with time slice " + std::to_string(tslice) + 
    "ms and rr_add to " + (addToEnd?"END":"BEGINNING"))) , false);  
  }
  std::string detail; 
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
      printEvent(switchingInProc -> nextState(timestamp, tcs), false); 
      
      resetBurstTimer();
      if (switchingInProc -> getState() != Process::State::RUNNING) {
        throw std::runtime_error("Error: SW_IN process did not switch to RUNNING state.");
      }
      runningProc = switchingInProc;
      switchingInProc = nullProc;
    } else if (switchingOutProc != nullProc) {
      
      if (switchingOutProc -> getState() == Process::State::SW_WAIT) {
        printEvent(switchingOutProc -> nextState(timestamp, tcs), false); 
        
        pushIo(switchingOutProc);
        switchingOutProc = nullProc;
      } else if (switchingOutProc -> getState() == Process::State::SW_READY) {
        printEvent(switchingOutProc -> nextState(timestamp, tcs), false);
        pushLastReady(switchingOutProc);
        switchingOutProc = nullProc;
      } else if (switchingOutProc -> getState() == Process::State::SW_TERM) {
        printEvent(switchingOutProc -> nextState(timestamp, tcs), true);
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
        printEvent(switchingInProc -> nextState(timestamp, tcs), false);
        if (switchingInProc -> getState() != Process::State::SW_IN) {
          throw std::runtime_error("Error: READY process did not switch to SW_IN state.");
        }
        resetTcsRemaining();
        if (!tcsRemaining) {
          return tick(); 
        }
        decrementTcs(); 
      } else if (latestProcessIdx == numProcs && ioQueue.empty()
        && switchingInProc == nullProc && switchingOutProc == nullProc) {
        printEvent("Simulator ended for " + std::string((fcfs?"FCFS":"RR")), true); 
        return false; // Simulation is over.
      }
    } else {
      ++cpuUsageTime; 
      if (runningProc -> getState() != Process::State::RUNNING) {
        throw std::runtime_error("Error: SW_IN process did not switch to RUNNING state.");
      }

      Process::State currState = decrementBurstTimer(); 
      if (currState == Process::State::RUNNING) {
        if (burstTimerElapsed()) {
          if (!isReadyQueueEmpty() && !fcfs) {
            preemptRunningProc(); 
            resetTcsRemaining();
            switchingOutProc = runningProc;
            
            runningProc = nullProc;
          } else {
            if (!fcfs) {
              ++timestamp;
              
                printEvent("Time slice expired; no preemption because ready queue is empty", false); 
              --timestamp;
            }
            resetBurstTimer(); 
          }
        }
      } else if (currState == Process::State::SW_WAIT) {
        ++timestamp;
        printEvent("Process " + std::string(1, runningProc -> getPid()) + " completed a CPU burst; " +
          std::to_string(runningProc -> getBurstsRemaining()) + " burst" + (runningProc -> getBurstsRemaining() == 1?" ":"s ") + "to go", false);
        --timestamp;
        resetTcsRemaining();
        switchingOutProc = runningProc;
        runningProc = nullProc;
        ++timestamp;
        printEvent("Process " + std::string(1, switchingOutProc -> getPid()) + 
          " switching out of CPU; will block on I/O until time " + std::to_string(switchingOutProc -> getCurrIoBurstTime() + timestamp + tcsRemaining) + "ms", false); 
        --timestamp;
      } else if (currState == Process::State::SW_TERM) {
        ++timestamp;
        printEvent("Process " + std::string(1, runningProc -> getPid()) + " terminated", true);
        --timestamp;
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
    detail = ioQueue.top().second -> nextState(timestamp, tcs); 
    if (ioQueue.top().second -> getState() != Process::State::READY) {
      throw std::runtime_error("Error: WAITING process did not switch to READY state.");
    }
    if (addToEnd) {
      pushLastReady(ioQueue.top().second);
    } else {
      pushFirstReady(ioQueue.top().second);
    }
    printEvent(detail, false);
    ioQueue.pop();
  }

  while(latestProcessIdx < numProcs &&
    orderedProcesses[latestProcessIdx] -> getArrivalTime() <= timestamp) {
    if (orderedProcesses[latestProcessIdx] -> getState() != Process::State::UNARRIVED) {
      throw std::runtime_error("Error: unarrived process was not in UNARRIVED state.");
    }
    detail = orderedProcesses[latestProcessIdx] -> nextState(timestamp, tcs);
    if (orderedProcesses[latestProcessIdx] -> getState() != Process::State::READY) {
      throw std::runtime_error("Error: UNARRIVED process did not switch to READY state.");
    }
    if (addToEnd) {
      pushLastReady(orderedProcesses[latestProcessIdx]);
    } else {
      pushFirstReady(orderedProcesses[latestProcessIdx]);
    }
    printEvent(detail, false);
    ++latestProcessIdx;
  }

  if (runningProc == nullProc && switchingInProc == nullProc && switchingOutProc == nullProc && !isReadyQueueEmpty()) {
    switchingInProc = peekFirstReady();
    if(switchingInProc -> getState() != Process::State::READY) {
      throw std::runtime_error("Error: process that was pulled from ready queue was not in READY state.");
    }
    popFirstReady();
    printEvent(switchingInProc -> nextState(timestamp, tcs), false);
    if (switchingInProc -> getState() != Process::State::SW_IN) {
      throw std::runtime_error("Error: READY process did not switch to SW_IN state.");
    }
    resetTcsRemaining();
    if (!tcsRemaining) {
      return tick(); 
    }
    decrementTcs();  
  }
  ++timestamp;
  return true;
}

double RoundRobin::calcAvgWaitTime() const {
  unsigned long long num = 0;
  unsigned long long den = 0;
  for (const auto & p : orderedProcesses) {
    auto data = p -> getTotalWaitTime();
    num += data.first;
    den += data.second;
  }
  return (double)num/den;
}
double RoundRobin::calcAvgTurnaroundTime() const {
  unsigned long long num = 0;
  unsigned long long den = 0;
  for (const auto & p : orderedProcesses) {
    auto data = p -> getTotalTurnaroundTime();
    num += data.first;
    den += data.second;
  }
  return (double)num/den;
}
double RoundRobin::calcAvgCpuBurstTime() const {
  unsigned long long num = 0;
  unsigned long long den = 0;
  for (const auto & p : orderedProcesses) {
    auto data = p -> getTotalCpuBurstTime();
    num += data.first;
    den += data.second;
  }
  return (double)num/den;
}
unsigned long long RoundRobin::calcTotalNumCtxSwitches() const {
  return std::accumulate(orderedProcesses.begin(), orderedProcesses.end(), 0,
    [](unsigned long long prev, const ProcessPtr& a) -> unsigned long long {
      return a -> getNumCtxSwitches() + prev;
    });
}
unsigned long long RoundRobin::calcTotalNumPreemptions() const {
  return std::accumulate(orderedProcesses.begin(), orderedProcesses.end(), 0,
    [](unsigned long long prev, const ProcessPtr& a) -> unsigned long long {
      return a -> getNumPreempts() + prev;
    });
}

void RoundRobin::printInfo(std::ostream& os) const {
  os << "Algorithm " + std::string(fcfs?"FCFS":"RR") << std::endl;

  os.precision(3);
  os << "-- average CPU burst time: " << std::fixed << calcAvgCpuBurstTime() << " ms" << std::endl;
  os << "-- average wait time: " << std::fixed << calcAvgWaitTime() << " ms" << std::endl;
  os << "-- average turnaround time: " << std::fixed << calcAvgTurnaroundTime() << " ms" << std::endl;
  
  os.precision(0);
  os << "-- total number of context switches: " << calcTotalNumCtxSwitches() << std::endl;
  os << "-- total number of preemptions: " << calcTotalNumPreemptions() << std::endl;

  os.precision(3);
  os << "-- CPU utilization: " << std::fixed << calcCpuUtilization() << "%" << std::endl;
  
}
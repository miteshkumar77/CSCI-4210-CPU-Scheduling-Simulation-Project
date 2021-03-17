#include "ShortestRemainingTime.hpp"



const std::function<bool(const ShortestRemainingTime::ProcessPtr&, const ShortestRemainingTime::ProcessPtr&)> 
ShortestRemainingTime::processArrivalComparator =
[] (const ShortestRemainingTime::ProcessPtr& a, const ShortestRemainingTime::ProcessPtr& b) -> bool {
  return a -> getArrivalTime() < b -> getArrivalTime() ||
    (a -> getArrivalTime() == b -> getArrivalTime() && a -> getPid() < b -> getPid()); 
};

const std::function<bool(const ShortestRemainingTime::ioQueueElem&, const ShortestRemainingTime::ioQueueElem&)>
ShortestRemainingTime::processIoComparator = 
[] (const ShortestRemainingTime::ioQueueElem& a, const ShortestRemainingTime::ioQueueElem& b) -> bool {
  return a.first > b.first || 
    (a.first == b.first && a.second -> getPid() > b.second -> getPid()); 
};

const std::function<bool(const ShortestRemainingTime::ProcessPtr&, const ShortestRemainingTime::ProcessPtr&)>
ShortestRemainingTime::readyQueueComparator = 
[] (const ShortestRemainingTime::ProcessPtr& left, const ShortestRemainingTime::ProcessPtr& right) -> bool {

  signed long int lv = left -> getExpectedRemainingBurstTime(); 
  signed long int rv = right -> getExpectedRemainingBurstTime();
  return lv > rv || (lv == rv && left -> getPid() > right -> getPid()); 
};

ShortestRemainingTime::ShortestRemainingTime(std::vector<Process>& procs,
  unsigned int tcs,
  bool sjf):
  ioQueue(processIoComparator),
  tcs(tcs), numProcs(procs.size()),
  runningProc(procs.end()),
  switchingOutProc(procs.end()),
  switchingInProc(procs.end()),
  nullProc(procs.end()),
  sjf(sjf) {
    orderedProcesses.reserve(procs.size()); 
    for (auto it = procs.begin(); it != procs.end(); ++it)
      orderedProcesses.push_back(it); 
    
    sort(orderedProcesses.begin(), orderedProcesses.end(), processArrivalComparator);
}

std::string ShortestRemainingTime::fmtProc(const ShortestRemainingTime::ProcessPtr& p) const {
  if (p == nullProc) {
    throw std::runtime_error("Error: fmtProc() called for null process ptr.");
  }
  return "Process " + std::string(1, p -> getPid()) + " (tau " + std::to_string(p -> getTau()) + "ms)"; 
}

std::string ShortestRemainingTime::fmtRecalcTau(const ShortestRemainingTime::ProcessPtr& p) const {
  if (p == nullProc) {
    throw std::runtime_error("Error: fmtRecalcTau() called for null process ptr.");
  }
  return "Recalculated tau (" + std::to_string(p -> getTau()) + "ms) for process " + std::string(1, p -> getPid()); 
}

void ShortestRemainingTime::reset() {

  if (!(switchingInProc == nullProc && 
        switchingOutProc == nullProc && 
        runningProc == nullProc && 
        isReadyQueueEmpty() && 
        latestProcessIdx >= orderedProcesses.size() &&
        ioQueue.empty())) {
    throw std::runtime_error("Error: ShortestRemainingTime::reset() called while algorithm finish constraints not satisfied.");
  }
  for (auto ptr : orderedProcesses) {
    ptr -> reset(); 
    timestamp = 0;
    latestProcessIdx = 0;
    burstRemaining = 0;
    cpuUsageTime = 0;
    orderedProcesses.clear(); 
    runningProc = nullProc;
    switchingOutProc = nullProc;
    switchingInProc = nullProc;
  }
}

void ShortestRemainingTime::decrementTcs() {
  if (switchingOutProc == nullProc && switchingInProc == nullProc) {
    throw std::runtime_error("Error: decrementTcs() when there isn't a process in context switch.");
  }
  if (!tcsRemaining) {
    throw std::runtime_error("Error: decrementTcs() called when tcsRemaining is 0.");
  }
  --tcsRemaining;
}


ShortestRemainingTime::ProcessPtr ShortestRemainingTime::peekFirstReady() const {
  if (isReadyQueueEmpty()) {
    throw std::runtime_error("Error: Attempted to peek first from an empty ready queue."); 
  }
  return readyQueue.front(); 
}


void ShortestRemainingTime::popFirstReady() {
  if (isReadyQueueEmpty()) {
    throw std::runtime_error("Error: Attempted to pop first from an empty ready queue.");
  }
  readyQueue.pop_front(); 
}



void ShortestRemainingTime::pushReady(ShortestRemainingTime::ProcessPtr processPtr) {
  if (processPtr -> getState() != Process::State::READY) {
    throw std::runtime_error("Error: pushFirstReady() called for process that wasn't in the READY state."); 
  }
  readyQueue.push_back(processPtr); 
  auto curr = readyQueue.rbegin(); 
  for (auto it = next(readyQueue.rbegin()); it != readyQueue.rend() && readyQueueComparator(*it, *curr); ++it, ++curr) {
    std::swap(*it, *curr); 
  }
}



Process::State ShortestRemainingTime::decrementBurstTimer() {
  if (runningProc == nullProc) {
    throw std::runtime_error("Error: decrementBurstTimer() was called without a process in the CPU.");
  }
  if (runningProc -> getState() != Process::State::RUNNING) {
    throw std::runtime_error("Error: decrementBurstTimer() was called without a process in RUNNING state.");
  }
  return runningProc -> decrementBurst(); 
}

void ShortestRemainingTime::resetTcsRemaining() {
  if (tcsRemaining) {
    throw std::runtime_error("Error: resetTcsRemaining() called when tcsRemaining was not 0.");
  }
  tcsRemaining = tcs/2;
}



void ShortestRemainingTime::preemptRunningProc() {
  if (runningProc == nullProc) {
    throw std::runtime_error("Error: preemptRunningProc() called when there was no runningProc.");
  }
  
  if (runningProc -> getState() != Process::State::RUNNING) {
    throw std::runtime_error("Error: preemptRunningProc() called for a non-RUNNING process.");
  }
  if (sjf) {
    throw std::runtime_error("Error: preemptRunningProc() calle for SJF.");
  }
  // printEvent("Time slice expired; process " + std::string(1, runningProc -> getPid()) + " preempted with " + std::to_string(runningProc -> getRemainingBurstTime()) + "ms to go", false);
  runningProc -> preempt();
}

void ShortestRemainingTime::pushIo(ProcessPtr processPtr) {
  if (processPtr -> getState() != Process::State::WAITING) {
    throw std::runtime_error("Error: pushIo() called for process that isn't in WAITING state.");
  }
  ioQueue.push({processPtr -> getCurrIoBurstTime() + timestamp, processPtr});
}

void ShortestRemainingTime::printEvent(const std::string& detail, bool term) const {
  if (!term && timestamp > MAX_OUTPUT_TS) return;
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

void ShortestRemainingTime::checkRep() const {
  if (runningProc != nullProc && switchingOutProc != nullProc) {
    throw std::runtime_error("Error: simultaneous runningProc and switchingOutProc.");
  }
  if (runningProc != nullProc && switchingInProc != nullProc) {
    throw std::runtime_error("Error: simultaneous runningProc and switchingInProc.");
  }
  if (switchingInProc != nullProc && switchingOutProc != nullProc) {
    throw std::runtime_error("Error: simultaneous switchingInProc and switchingOutProc.");
  }
  if (tcsRemaining && switchingInProc == nullProc && switchingOutProc == nullProc) {
    throw std::runtime_error("Error: no switchingInProc or switchingOutProc while tcsRemaining.");
  }
}

void ShortestRemainingTime::run() {
  checkRep();
  for (char it = 'A'; it <= 'A' + orderedProcesses.size() - 1; ++it) {
    for (auto& proc : orderedProcesses) {
      if (proc -> getPid() == it) {
        std::cout << "Process " << it << " [NEW] (arrival time " << proc -> getArrivalTime() << " ms) ";
        std::cout << proc -> getNumBursts() << " CPU burst" << (proc -> getNumBursts() == 1?" ":"s ");
        std::cout << "(tau " << proc -> getTau() << "ms)" << std::endl;
      }
    }
  }
  printEvent("Simulator started for " + std::string((sjf?"SJF":"SRT")), false);

  std::string detail;

  while(true) { // <<< BEGIN SRT/SJF
    checkRep();
    if (switchingInProc == nullProc &&
        switchingOutProc == nullProc &&
        runningProc == nullProc &&
        isReadyQueueEmpty() &&
        latestProcessIdx >= orderedProcesses.size() &&
        ioQueue.empty()) {
      
      break;
    }
    
    // std::cout << (int)(switchingOutProc == nullProc) << ' '
    //   << (int)(switchingInProc == nullProc) << ' '
    //   << (int)(runningProc == nullProc) << ' '
    //   << (int)(isReadyQueueEmpty()) << ' '
    //   << (int)(latestProcessIdx >= orderedProcesses.size()) << ' '
    //   << (int)ioQueue.empty() << ' '
    //   << timestamp << ' '
    //   << ioQueue.size() << std::endl;

    if (tcsRemaining) {
      // decrement context switch timer
      decrementTcs();
    }

    // A
    if (!tcsRemaining && switchingOutProc != nullProc) {
      if (switchingOutProc -> getState() == Process::State::SW_WAIT) {
        switchingOutProc -> nextState(timestamp, tcs);
        if (switchingOutProc -> getState() != Process::State::WAITING) {
          throw std::runtime_error("Error: switchingOutProcess in SW_WAIT did not switch to WAITING state.");
        }
        pushIo(switchingOutProc);
      } else if (switchingOutProc -> getState() == Process::State::SW_TERM) {
        switchingOutProc -> nextState(timestamp, tcs);
        if (switchingOutProc -> getState() != Process::State::TERMINATED) {
          throw std::runtime_error("Error: switchingOutProcess in SW_TERM did not switch to TERMINATED state.");
        }        
      } else if (switchingOutProc -> getState() == Process::State::SW_READY) {
        switchingOutProc -> nextState(timestamp, tcs);
        pushReady(switchingOutProc);
        if (switchingOutProc -> getState() != Process::State::READY) {
          throw std::runtime_error("Error: switchingOutProcess in SW_READY did not switch to READY state.");
        } 
      } else {
        throw std::runtime_error("Error: switching out process was not in SW_WAIT, SW_READY, or SW_TERM state.");
      }
      switchingOutProc = nullProc;
    }

    
    // B
    // (a) CPU burst completion
    if (runningProc != nullProc) {
      
      if (runningProc -> getState() != Process::State::RUNNING) {
        throw std::runtime_error("Error: SW_IN process did not switch to RUNNING state");
      }
      ++cpuUsageTime;
      std::string label = fmtProc(runningProc); 
      Process::State currState = decrementBurstTimer();
      if (currState == Process::State::RUNNING) {
        // Nothing to be done here
      } else if (currState == Process::State::SW_WAIT) {
        // completed a CPU burst

        printEvent(label + " completed a CPU burst; " + 
            std::to_string(runningProc -> getBurstsRemaining()) + " burst" + (runningProc -> getBurstsRemaining() == 1?" ":"s ") + "to go", false); 
        printEvent(fmtRecalcTau(runningProc), false);
        resetTcsRemaining();
        switchingOutProc = runningProc;
        runningProc = nullProc;
        // Go to I/O
        printEvent("Process " + std::string(1, switchingOutProc -> getPid()) + 
          " switching out of CPU; will block on I/O until time " + std::to_string(switchingOutProc -> getCurrIoBurstTime() + timestamp + tcsRemaining) + "ms", false);
      } else if (currState == Process::State::SW_TERM) {
        // Terminated!
        printEvent("Process " + std::string(1, runningProc -> getPid()) + " terminated", true);
        resetTcsRemaining();
        switchingOutProc = runningProc;
        runningProc = nullProc;
      } else {
        throw std::runtime_error("Error: runningProc wasn't in RUNNING, SW_WAIT, or SW_TERM stage after decrementBurst"); 
      }
    }

    // C
    if (!tcsRemaining && switchingInProc != nullProc) {
      if (switchingInProc -> getState() != Process::State::SW_IN) {
        throw std::runtime_error("Error: Switching in process did not have correct SW_IN process state.");
      }
      printEvent(fmtProc(switchingInProc) + " started using the CPU with " + 
        std::to_string(switchingInProc -> getRemainingBurstTime()) + "ms burst remaining", false);
      switchingInProc -> nextState(timestamp, tcs);
      if (switchingInProc -> getState() != Process::State::RUNNING) {
        throw std::runtime_error("Error: SW_IN process did not switch to RUNNING state.");
      }
      runningProc = switchingInProc;
      switchingInProc = nullProc;
    }

    // D
    if (!sjf && runningProc != nullProc &&
        !isReadyQueueEmpty() &&
          peekFirstReady() -> getExpectedRemainingBurstTime() < runningProc -> getExpectedRemainingBurstTime()) {
        printEvent(fmtProc(peekFirstReady()) + " will preempt " + std::string(1, runningProc -> getPid()), false);
      preemptRunningProc(); 
      resetTcsRemaining(); 
      switchingOutProc = runningProc;
      runningProc = nullProc;
    }


    // E
    // (b) I/O burst completions
    while(!ioQueue.empty() && ioQueue.top().first <= timestamp) {
      if (ioQueue.top().second -> getState() != Process::State::WAITING) {
        throw std::runtime_error("Error: process in ioQueue was not in WAITING state.");
      }
      detail = ioQueue.top().second -> nextState(timestamp, tcs); 
      if (ioQueue.top().second -> getState() != Process::State::READY) {
        throw std::runtime_error("Error: UNARRIVED process did not switch to READY state.");
      }

      pushReady(ioQueue.top().second);
    
      if (!sjf && runningProc != nullProc && 
          switchingOutProc == nullProc &&
          runningProc -> getExpectedRemainingBurstTime() > ioQueue.top().second -> getExpectedRemainingBurstTime()) {
        // arriving process preempts running process
        
        preemptRunningProc(); 
        resetTcsRemaining();
        switchingOutProc = runningProc;
        runningProc = nullProc;
        if (switchingOutProc -> getState() != Process::State::SW_READY) {
          throw std::runtime_error("Error: preempted process in RUNNING state did not move into SW_READY state");
        }
        printEvent(fmtProc(ioQueue.top().second) + " completed I/O; preempting " + std::string(1, switchingOutProc -> getPid()), false); 
        
      } else {
        // Finished I/O, back to ready queue
        printEvent(fmtProc(ioQueue.top().second) + " completed I/O; placed on ready queue", false);
        
        
      }
      ioQueue.pop(); 
    }

    
    
    // F
    // (c) New process arrivals
    while(latestProcessIdx < numProcs &&
      orderedProcesses[latestProcessIdx] -> getArrivalTime() <= timestamp) {
      if (orderedProcesses[latestProcessIdx] -> getState() != Process::State::UNARRIVED) {
        throw std::runtime_error("Error: unarrived process was not in UNARRIVED state.");
      }
      detail = orderedProcesses[latestProcessIdx] -> nextState(timestamp, tcs);
      if (orderedProcesses[latestProcessIdx] -> getState() != Process::State::READY) {
        throw std::runtime_error("Error: UNARRIVED process did not switch to READY state.");
      }

      pushReady(orderedProcesses[latestProcessIdx]);
      if (!sjf && runningProc != nullProc && 
          switchingOutProc == nullProc &&
          runningProc -> getExpectedRemainingBurstTime() > orderedProcesses[latestProcessIdx] -> getExpectedRemainingBurstTime()) {        
        // Newly arrived process preempted running process
        preemptRunningProc(); 
        switchingOutProc = runningProc;
        runningProc = nullProc;
        if (switchingOutProc -> getState() != Process::State::SW_READY) {
          throw std::runtime_error("Error: preempted process in RUNNING state did not move into SW_READY state");
        }
        resetTcsRemaining();
        printEvent(fmtProc(orderedProcesses[latestProcessIdx]) + " arrived; preempting " + std::string(1, switchingOutProc -> getPid()), false);
      } else {
        printEvent(fmtProc(orderedProcesses[latestProcessIdx]) + " arrived; placed on ready queue", false);
      }
      ++latestProcessIdx;
    }
    

    // A B C E F G

    // G
    if (!tcsRemaining && runningProc == nullProc && switchingInProc == nullProc && switchingOutProc == nullProc && !isReadyQueueEmpty()) {
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
      
    }

    // if (timestamp == 17408) {
    //   if (switchingOutProc != nullProc) {
    //     std::cout << "Switching Out Proc: " << 
    //   }  
    // }

    
    
    ++timestamp;
    checkRep();
  } // <<< END SRT/SJF

  --timestamp;
  printEvent("Simulator ended for " + std::string((sjf?"SJF":"SRT")), true); 

}

double ShortestRemainingTime::calcAvgWaitTime() const {
  unsigned long long num = 0;
  unsigned long long den = 0;
  for (const auto & p : orderedProcesses) {
    auto data = p -> getTotalWaitTime();
    num += data.first;
    den += data.second;
  }
  return (double)num/den;
}
double ShortestRemainingTime::calcAvgTurnaroundTime() const {
  unsigned long long num = 0;
  unsigned long long den = 0;
  for (const auto & p : orderedProcesses) {
    auto data = p -> getTotalTurnaroundTime();
    num += data.first;
    den += data.second;
  }
  return (double)num/den;
}
double ShortestRemainingTime::calcAvgCpuBurstTime() const {
  unsigned long long num = 0;
  unsigned long long den = 0;
  for (const auto & p : orderedProcesses) {
    auto data = p -> getTotalCpuBurstTime();
    num += data.first;
    den += data.second;
  }
  return (double)num/den;
}
unsigned long long ShortestRemainingTime::calcTotalNumCtxSwitches() const {
  return std::accumulate(orderedProcesses.begin(), orderedProcesses.end(), 0,
    [](unsigned long long prev, const ProcessPtr& a) -> unsigned long long {
      return a -> getNumCtxSwitches() + prev;
    });
}
unsigned long long ShortestRemainingTime::calcTotalNumPreemptions() const {
  return std::accumulate(orderedProcesses.begin(), orderedProcesses.end(), 0,
    [](unsigned long long prev, const ProcessPtr& a) -> unsigned long long {
      return a -> getNumPreempts() + prev;
    });
}

void ShortestRemainingTime::printInfo(std::ostream& os) const {
  os << "Algorithm " + std::string(sjf?"SJF":"SRT") << std::endl;

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
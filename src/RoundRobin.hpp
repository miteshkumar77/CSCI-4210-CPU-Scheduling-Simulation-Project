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
  public:
    RoundRobin(std::vector<Process>& processes, unsigned int tslice, unsigned int tcs); 
    bool tick(); 
    void printInfo() const;
  private:
    typedef std::vector<Process>::iterator ProcessPtr;
    typedef std::pair<unsigned int, ProcessPtr> ioQueueElem;


    bool isReadyQueueEmpty() const { return readyQueue.empty(); }
    ProcessPtr peekLastReady() const;
    ProcessPtr peekFirstReady() const;
    void popFirstReady(); 
    void pushLastReady(ProcessPtr processPtr);
    void pushFirstReady(ProcessPtr processPtr);
    Process::State decrementBurstTimer(); 
    void pushIo(ProcessPtr processPtr); 

    void resetTcsRemaining(); 
    void resetBurstTimer();
    
    void decrementTcs();
    void preemptRunningProc();
    inline bool burstTimerElapsed() const { return burstRemaining == 0; }

    // Independent
    static const std::function<bool(const ProcessPtr&, const ProcessPtr&)> processArrivalComparator; 
    static const std::function<bool(const ioQueueElem&, const ioQueueElem&)> processIoComparator;
    std::deque<ProcessPtr> readyQueue;
    int latestProcessIdx = 0;
    int timestamp = 0;
    unsigned int burstRemaining = 0;
    unsigned int tcsRemaining = 0;
    const bool addToEnd = true;
    unsigned int numPreempts = 0;
    unsigned int numCtxSwitches = 0;

    // Default
    std::priority_queue<ioQueueElem, std::vector<ioQueueElem>, decltype(processIoComparator)> ioQueue;
    const unsigned int tslice;
    const unsigned int tcs;
    const unsigned int numProcs;
    ProcessPtr runningProc;
    ProcessPtr switchingOutProc;
    ProcessPtr switchingInProc;
    const ProcessPtr nullProc;

    // Non-Default
    std::vector<ProcessPtr> orderedProcesses;
};

#endif
#ifndef SHORTESTREMAININGTIME_HPP
#define SHORTESTREMAININGTIME_HPP

#include <deque>
#include <queue>
#include <algorithm>
#include <vector>
#include <functional>
#include <exception>
#include <stdexcept>
#include "Process.hpp"
#include <utility>
#include <string>
#include <sstream>
#include <iostream>


class ShortestRemainingTime {
  public:
    ShortestRemainingTime(std::vector<Process>& processes, unsigned int tcs, bool sjf); 
    void printInfo(std::ostream& os) const;
    void run(); 
    void reset();
  private:
    typedef std::vector<Process>::iterator ProcessPtr;
    typedef std::pair<unsigned int, ProcessPtr> ioQueueElem;
    void printEvent(const std::string& detail, bool term) const; 
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
    double calcCpuUtilization() const { return 100.0 * (double)cpuUsageTime/timestamp; }
    void resetTcsRemaining(); 
    void checkRep() const;
    void decrementTcs();
    void preemptRunningProc();
    inline bool burstTimerElapsed() const { return burstRemaining == 0; }
    std::string fmtProc(const ProcessPtr& p) const;
    std::string fmtRecalcTau(const ShortestRemainingTime::ProcessPtr& p) const;

    // Independent
    static const std::function<bool(const ProcessPtr&, const ProcessPtr&)> processArrivalComparator; 
    static const std::function<bool(const ioQueueElem&, const ioQueueElem&)> processIoComparator;
    static const std::function<bool(const ProcessPtr&, const ProcessPtr&)> readyQueueComparator;
    std::deque<ProcessPtr> readyQueue;
    int latestProcessIdx = 0;
    int timestamp = 0;
    unsigned int burstRemaining = 0;
    unsigned int tcsRemaining = 0;
    
    unsigned int cpuUsageTime = 0;

    // Default  
    std::priority_queue<ioQueueElem, std::vector<ioQueueElem>, decltype(processIoComparator)> ioQueue;
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
/**
 *  CSCI 4210 Operating Systems
 *  2021 Spring
 * 
 *  Simulation Project - RoundRobin.hpp
 * 
 *  Authors:
 *    Mitesh Kumar  [ kumarm4 ]
 *    Jason Lam     [  lamj7  ]
 *    William He    [  hew7   ]
 * 
 *  Brief:
 *    Implementation of Round Robin (RR) scheduling algorithm that can be converted to 
 *    First-Come-First-Served (FCFS) by passing fcfs=true to the constructor.
 *    
 *    run() runs the simulation and outputs important events to terminal in chronological order
 *    printInfo() can be called successfully after run() has completed and will print the final calculated statistics 
 *                of the run to a output stream. 
 *    reset() resets the object for a future run and resets the internal processes to their initial state.
 */

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
#include <string>
#include <sstream>
#include <iostream>


class RoundRobin {
  public:
    RoundRobin(std::vector<Process>& processes, unsigned int tslice, unsigned int tcs, bool addToEnd, bool fcfs); 
    void printInfo(std::ostream& os) const;
    void run(); 
    void reset();
  private:
    typedef std::vector<Process>::iterator ProcessPtr;
    typedef std::pair<unsigned int, ProcessPtr> ioQueueElem;

    void printEvent(const std::string& detail, bool term) const; 
    bool isReadyQueueEmpty() const { return readyQueue.empty(); }
    ProcessPtr peekLastReady() const;
    ProcessPtr peekFirstReady() const;
    void popFirstReady(); 
    void pushLastReady(ProcessPtr processPtr);
    void pushFirstReady(ProcessPtr processPtr);
    Process::State decrementBurstTimer(); 
    void pushIo(ProcessPtr processPtr); 

    double calcAvgWaitTime() const;
    double calcAvgTurnaroundTime() const;
    double calcAvgCpuBurstTime() const;
    unsigned long long calcTotalNumCtxSwitches() const;
    unsigned long long calcTotalNumPreemptions() const;
    double calcCpuUtilization() const { return 100.0 * (double)cpuUsageTime/timestamp; }
    void resetTcsRemaining(); 
    void resetBurstTimer();
    
    void decrementTcs();
    void preemptRunningProc();
    inline bool burstTimerElapsed() const { return burstRemaining == 0; }

    // Independent
    static const std::function<bool(const ProcessPtr&, const ProcessPtr&)> processArrivalComparator; 
    static const std::function<bool(const ioQueueElem&, const ioQueueElem&)> processIoComparator;
    std::deque<ProcessPtr> readyQueue;
    unsigned int latestProcessIdx = 0;
    unsigned int timestamp = 0;
    unsigned int burstRemaining = 0;
    unsigned int tcsRemaining = 0;
    
    unsigned int cpuUsageTime = 0;

    // Default
    std::priority_queue<ioQueueElem, std::vector<ioQueueElem>, decltype(processIoComparator)> ioQueue;
    const unsigned int tslice;
    const unsigned int tcs;
    const unsigned int numProcs;
    ProcessPtr runningProc;
    ProcessPtr switchingOutProc;
    ProcessPtr switchingInProc;
    const ProcessPtr nullProc;
    const bool addToEnd;
    const bool fcfs;

    // Non-Default
    std::vector<ProcessPtr> orderedProcesses;
};

#endif
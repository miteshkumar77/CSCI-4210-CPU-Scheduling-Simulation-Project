#include <stdlib.h>
#include <iostream>
#include <limits>
#include "SeqGenerator.hpp"
#include "Process.hpp"
#include "RoundRobin.hpp"

int main(int argc, char** argv) {

  // std::vector<Process> processes = SeqGenerator::parseProcesses("test_input.txt"); 
  std::vector<Process> processes = SeqGenerator::generateProccesses(26, 0.001, 3000, 42);
  RoundRobin rr(processes, /* tslice: */ 3, /* tcs: */ 5); 
  std::cout << "Beginning Simulation..." << std::endl;

  while(rr.tick()); 
  rr.printInfo(); 

  // for (auto& process : processes) process.printInfo(); 
  return EXIT_SUCCESS; 
}
#include <stdlib.h>
#include <iostream>
#include <limits>
#include "SeqGenerator.hpp"
#include "Process.hpp"
#include "RoundRobin.hpp"

int main(int argc, char** argv) {

  std::vector<Process> processes = SeqGenerator::parseProcesses("test_input.txt"); 
  RoundRobin rr(processes, /* tslice: */ 3, /* tcs: */ 0); 

  std::cout << "Beginning simulation..." << std::endl;
  for (int i = 0; i < std::numeric_limits<int>::max(); ++i) {
    if (!rr.tick()) {
      std::cout << "RR Simulation ended after " << i << "ms." << std::endl;
      break;
    }
  }

  for (auto& process : processes) process.printProcess(); 
  return EXIT_SUCCESS; 
}
#include <stdlib.h>
#include <iostream>
#include <limits>
#include "SeqGenerator.hpp"
#include "Process.hpp"
#include "RoundRobin.hpp"

int main(int argc, char** argv) {

  // std::vector<Process> processes = SeqGenerator::parseProcesses("test_input_2.txt"); 
  std::vector<Process> processes = SeqGenerator::generateProccesses(10, 0.01, 400, 4211);
  RoundRobin rr(processes, /* tslice: */ 4, /* tcs: */ 3); 
  std::cout << "Beginning Simulation..." << std::endl;

  while(rr.tick()); 
  rr.printInfo(); 

  // for (auto& process : processes) process.printInfo(); 
  return EXIT_SUCCESS; 
}
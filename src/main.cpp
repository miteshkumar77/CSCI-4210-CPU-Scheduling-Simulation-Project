#include <stdlib.h>
#include <iostream>
#include <limits>
#include "SeqGenerator.hpp"
#include "Process.hpp"
#include "RoundRobin.hpp"

int main(int argc, char** argv) {

  std::vector<Process> processes = SeqGenerator::parseProcesses("test_input.txt"); 
  // std::vector<Process> processes = SeqGenerator::generateProccesses(26, 0.01, 300, 123);
  RoundRobin rr(processes, /* tslice: */ 3, /* tcs: */ 2); 
  std::cout << "Beginning Simulation..." << std::endl;
  // for (auto& p : processes) {
  //   p.makeEntry(); 
  // }
  while(rr.tick()); 
  rr.printInfo(); 

  for (auto& process : processes) process.printInfo(); 
  return EXIT_SUCCESS; 
}
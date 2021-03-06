#include <stdlib.h>
#include <iostream>
#include "SeqGenerator.hpp"
#include "Process.hpp"
#include "RoundRobin.hpp"

int main(int argc, char** argv) {

  std::vector<Process> processes = SeqGenerator::generateProccesses(5, 0.01, 10); 
  RoundRobin rr(processes, 1, 2); 

  std::cout << "Beginning simulation..." << std::endl;
  for (int i = 0; i < 1000000; ++i) {
    if (!rr.tick()) {
      std::cout << "RR Simulation ended after " << i << "ms." << std::endl;
      return EXIT_SUCCESS;
    }
  }

  std::cout << "RR Simulation didn't end after " << 100000 << "ms." << std::endl;
  for (auto& process : processes) process.printProcess(); 
  return EXIT_SUCCESS; 
}
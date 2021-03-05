#include <stdlib.h>
#include <iostream>
#include "SeqGenerator.hpp"
#include "Process.hpp"
#include "RoundRobin.hpp"

int main(int argc, char** argv) {

  std::vector<Process> processes = SeqGenerator::generateProccesses(26, 0.01, 3000); 
  for (auto& process : processes) process.printProcess(); 
  return EXIT_SUCCESS; 
}
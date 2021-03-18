#include <stdlib.h>
#include <iostream>
#include <limits>
#include <string>
#include <fstream>
#include "SeqGenerator.hpp"
#include "Process.hpp"
#include "RoundRobin.hpp"
#include "ShortestRemainingTime.hpp"

/* [n: number of processes] [seed] [lambda] [limit] [tcs] [alpha] [tslice] [rr_add: BEGINNING or END] */
int main(int argc, char** argv) {
  
  if (argc != 8 && argc != 9) {
    std::cerr << "ERROR: incorrect argc." << std::endl;
    return EXIT_FAILURE;
  }

  bool addToEnd = true;
  int n = atoi(*(argv + 1)); 
  long seedval = atol(*(argv + 2));
  double lambda = std::stod(*(argv + 3));
  int maxval = atoi(*(argv + 4)); 
  int tcs = atoi(*(argv + 5)); 
  double alpha = std::stod(*(argv + 6));
  int tslice = atoi(*(argv + 7)); 
  if (tcs % 2 != 0 && tcs <= 0) {
    std::cerr << "ERROR: tcs isn't a positive even number." << std::endl;
    return EXIT_FAILURE;
  }

  if (tslice <= 0) {
    std::cerr << "ERROR: tslice isn't a positive number." << std::endl;
    return EXIT_FAILURE;
  }

  if (argc == 9) {
    if ((*(argv + 8)) == std::string("BEGINNING")) {
      addToEnd = false;
    } else if ((*(argv + 8)) == std::string("END")) {
      /* add_to_end = true; */
    } else {
      std::cerr << "ERROR: rr_add (arg 8) must be either BEGINNING or END if included." << std::endl;
      return EXIT_FAILURE;
    }
  }

  std::ofstream ofs;
  ofs.open("simout.txt", std::ofstream::out | std::ofstream::trunc);

  std::vector<Process> processes = SeqGenerator::generateProccesses(n, lambda, maxval, seedval, alpha); 

  // FCFS
  RoundRobin fcfs(processes, tslice, tcs, addToEnd, /* FCFS: true */ true);
  fcfs.run(); 
  fcfs.printInfo(ofs);
  fcfs.reset();
  std::cout << std::endl;

  // SJF
  ShortestRemainingTime sjf(processes, tcs, /* SJF: true */ true);
  sjf.run();
  sjf.printInfo(ofs);
  sjf.reset();
  std::cout << std::endl;
  
  // SRT
  ShortestRemainingTime srt(processes, tcs, /* SRT: false */ false); 
  srt.run();
  srt.printInfo(ofs);
  srt.reset(); 
  std::cout << std::endl;

  // RR
  RoundRobin rr(processes, tslice, tcs, addToEnd, /* RR: false */false); 
  rr.run();  
  rr.printInfo(ofs); 
  rr.reset(); 

  ofs.close(); 
  return EXIT_SUCCESS; 
}


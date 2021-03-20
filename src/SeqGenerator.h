/**
 *  CSCI 4210 Operating Systems
 *  2021 Spring
 * 
 *  Simulation Project - SeqGenerator.hpp
 * 
 *  Authors:
 *    Mitesh Kumar  [ kumarm4 ]
 *    Jason Lam     [  lamj7  ]
 *    William He    [  hew7   ]
 */

#ifndef SEQGENERATOR_HPP
#define SEQGENERATOR_HPP

#include <stdlib.h>
#include <vector>
#include <math.h>
#include "Process.h"
#include <limits>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

namespace SeqGenerator {
  
  inline double nextExp(double lambda); 
  inline unsigned int randNumBursts(); 
  unsigned int floorNextExp(double lambda, double maxval);
  unsigned int ceilNextExp(double lambda, double maxval);
  std::vector<Process> generateProccesses(unsigned short n, double lambda, 
    double maxval, long int seedval, double alpha);
  std::vector<Process> parseProcesses(std::string fname, double lambda, unsigned int tcs, double alpha, unsigned int tslice); 
}

#endif
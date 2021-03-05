#ifndef SEQGENERATOR_HPP
#define SEQGENERATOR_HPP

#include <stdlib.h>
#include <vector>
#include <math.h>
#include "Process.hpp"
#include <limits>

namespace SeqGenerator {
  
  inline void seed(long int seedval) { srand48(seedval); }
  inline double nextExp(double lambda); 
  inline unsigned int randNumBursts(); 
  unsigned int floorNextExp(double lambda, double maxval);
  unsigned int ceilNextExp(double lambda, double maxval);
  std::vector<Process> generateProccesses(unsigned short n, 
    double lambda, double maxval); 
}

#endif
#ifndef SEQGENERATOR_HPP
#define SEQGENERATOR_HPP

#include <stdlib.h>
#include <vector>
#include <math.h>
#include "Process.hpp"
#include <limits>

namespace SeqGenerator {
  
  inline double nextExp(double lambda); 
  inline unsigned int randNumBursts(); 
  unsigned int floorNextExp(double lambda, double maxval);
  unsigned int ceilNextExp(double lambda, double maxval);
  std::vector<Process> generateProccesses(unsigned short n, double lambda, 
    double maxval, long int seedval);
}

#endif
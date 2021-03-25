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

#include "Process.hpp"
#include <fcntl.h>
#include <limits>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace SeqGenerator {

inline double nextExp(double lambda);
inline unsigned int randNumBursts();
unsigned int floorNextExp(double lambda, double maxval);
unsigned int ceilNextExp(double lambda, double maxval);
std::vector<Process> generateProccesses(unsigned short n, double lambda,
                                        double maxval, long int seedval,
                                        double alpha);
std::vector<Process> parseProcesses(std::string fname, double lambda,
                                    unsigned int tcs, double alpha,
                                    unsigned int tslice);
} // namespace SeqGenerator

#endif

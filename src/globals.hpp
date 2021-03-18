/**
 *  CSCI 4210 Operating Systems
 *  2021 Spring
 * 
 *  Simulation Project - globals.hpp
 * 
 *  Authors:
 *    Mitesh Kumar  [kumarm4]
 *    Jason Lam     [ lamj7 ]
 *    William He    [ hew7  ]
 * 
 *  Brief: 
 *    Place to store any global constants defined at compile time (e.g. DISPLAY_MAX_T)
 */

#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <limits>

#ifndef DISPLAY_MAX_T
#define MAX_OUTPUT_TS std::numeric_limits<unsigned int>::max()
#else
#define MAX_OUTPUT_TS DISPLAY_MAX_T
#endif

#endif
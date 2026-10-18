#include "menu.hpp"
#include "subsystems.hpp"

#pragma once
#ifndef PIDTURNCAL_HPP
#define PIDTURNCAL_HPP

/* Goal of this project:
 * 
 * I want to make an algorithm that detects offset in turning pid and uses a hill-climb algorithm (or a better one for PID)
 * to fine-tune my turning pid constants. 
*/

/* Goal of this file:
 * 
 * This is designed to be the header file where a function call can be placed. I will call it in main.cpp and run it there, but all
 * logic and functions for it will be in the pidturncal.cpp file.
*/

extern void calibratePID();

#endif
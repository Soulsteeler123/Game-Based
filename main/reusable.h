#pragma once
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#include <thread>

//Gets bit
#define BIT(a, n) ((a & (1 << n)) ? 1 : 0)

//Sets bit
#define BIT_SET(a, n, on) { if (on) a |= (1 << n); else a &= ~(1 << n); }

//Checks if number of indeterminate bases is between two other numbers
#define BETWEEN(a, b, c) ((a >= b) && (a >= c))

//Used to define that a function is not yet implemented
#define NOT_IMPL { std::cout << "Not yet implemented." << std::endl; exit(-1); }

//Delays program when needed
void delay(unsigned long ms);
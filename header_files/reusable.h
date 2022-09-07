#pragma once
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
//Gets bit
#define BIT(a, n) ((a & (1 << n)) ? 1 : 0)
//Sets bit
#define BIT_SET(a, n, on) (on ? (a) |= (1 << n) : (a) &= ~(1 << n))
//Checks if number of indeterminate bases is between two other numbers
#define BETWEEN(a, b, c) ((a >= b) && (a >= c))
//Delays program when needed
void delay(uint32_t ms);
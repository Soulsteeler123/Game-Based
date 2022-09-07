#pragma once
#include "reusable.h"

//Reads from the bus
uint8_t bus_read(uint16_t address);
//Writes to the bus
void bus_write(uint16_t address, uint8_t value);
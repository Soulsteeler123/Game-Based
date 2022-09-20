#pragma once
#include "../reusable.h"

//Reads from the bus
unsigned char bus_read(unsigned short address);
//Writes to the bus
void bus_write(unsigned short address, unsigned char value);

unsigned short bus_read16(unsigned short address);
void bus_write16(unsigned short address, unsigned short value);
#pragma once
#include "../reusable.h"

//Reads ram address
unsigned char bus_read(unsigned short address);
//Writes with ram address
void bus_write(unsigned short address, unsigned char value);

//Reads 16 bit ram address 
unsigned short bus_read16(unsigned short address);
//Writes with 16 bit ram address
void bus_write16(unsigned short address, unsigned short value);
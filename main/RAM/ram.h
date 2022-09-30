#pragma once

//Reads from the working ram based on address
unsigned char wram_read(unsigned short address);
//Writes to the working ram based on address with given value
void wram_write(unsigned short address, unsigned char value);

//Reads from the high ram based on address
unsigned char hram_read(unsigned short address);
//Writes to the high ram based on address with given value
void hram_write(unsigned short address, unsigned char value);
#pragma once

unsigned char wram_read(unsigned short address);
void wram_write(unsigned short address, unsigned char value);

unsigned char hram_read(unsigned short address);
void hram_write(unsigned short address, unsigned char value);
#include "ram.h"

//Contains arrays for the address of the ram
struct ram_context {
    unsigned char wram[0x2000];
    unsigned char hram[0x80];
};

//Context for the ram
static ram_context context;

unsigned char wram_read(unsigned short address) {
    //Returns value of working ram at given address offset by 0xC000
    return context.wram[address - 0xC000];
}

void wram_write(unsigned short address, unsigned char value) {
    //Writes value to the working ram at given address offset by 0xC000
    context.wram[address - 0xC000] = value;
}

unsigned char hram_read(unsigned short address) {
    //Reads value of high ram at given address offset by 0xFF80
    return context.wram[address - 0xFF80];
}

void hram_write(unsigned short address, unsigned char value) {
    //Writes value to the high ram at given address offset by 0xFF80
    context.wram[address - 0xFF80] = value;
}
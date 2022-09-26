#include "ram.h"

struct ram_context {
    unsigned char wram[0x2000];
    unsigned char hram[0x80];
};

static ram_context context;

unsigned char wram_read(unsigned short address) {
    return context.wram[address - 0xC000];
}

void wram_write(unsigned short address, unsigned char value) {
    context.wram[address - 0xC000] = value;
}

unsigned char hram_read(unsigned short address) {
    return context.wram[address - 0xFF80];
}

void hram_write(unsigned short address, unsigned char value) {
    context.wram[address - 0xFF80] = value;
}
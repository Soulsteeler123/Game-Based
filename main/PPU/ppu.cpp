#include "ppu.h"

static ppu_context context;

void ppu_init() {

}
void ppu_tick() {
    
}

void ppu_oam_write(unsigned short address, unsigned char value) {
    if(address >= 0xFE00)
        address -= 0xFE00;
    
    unsigned char *pointer = (unsigned char *)context.oam_ram;
    pointer[address] = value;
}

unsigned char ppu_oam_read(unsigned short address) {
    if(address >= 0xFE00)
        address -= 0xFE00;
    
    unsigned char *pointer = (unsigned char *)context.oam_ram;
    return pointer[address];
}

void ppu_vram_write(unsigned short address, unsigned char value) {
    context.vram[address - 0x8000] = value;
}

unsigned char ppu_vram_read(unsigned short address) {
    return context.vram[address - 0x8000];
}
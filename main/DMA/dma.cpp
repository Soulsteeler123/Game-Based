#include "dma.h"
#include "../PPU/ppu.h"
#include "../BUS/bus.h"

static dma_context context;

void dma_start(unsigned char start) {
    context.active = true;
    context.byte = 0;
    context.delay = 2;
    context.value = start;
}

void dma_tick() {
    if(!context.active) 
        return;
    else if (context.delay) {
        context.delay--;
        return;
    }

    ppu_oam_write(context.byte, bus_read((context.value * 0x100) + context.byte));

    context.byte++;
    context.active = context.byte < 0xA0;
    
}

bool dma_transfer() {
    return context.active;
}
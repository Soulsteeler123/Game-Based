#include "dma.h"
#include "../PPU/ppu.h"
#include "../BUS/bus.h"

static dma_context context;

void dma_start(unsigned char start) {
    //Sets default values of dma
    context.active = true;
    context.byte = 0;
    context.delay = 2;
    context.value = start;
}

void dma_tick() {
    if(!context.active) 
        return;
    //If there is a delay, decrements the delay and waits until there is no delay
    else if (context.delay) {
        context.delay--;
        return;
    }

    //Writes to the ppu's oam
    ppu_oam_write(context.byte, bus_read((context.value * 0x100) + context.byte));

    //Increments the current byte
    context.byte++;
    //Determines if dma is still active depending if its less then A0
    context.active = context.byte < 0xA0;
    
}

bool dma_transfer() {
    return context.active;
}
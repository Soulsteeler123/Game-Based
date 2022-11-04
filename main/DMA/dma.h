#pragma once

#include "../reusable.h"

struct dma_context {
    //If the dma is active
    bool active;
    //Current byte of the dma
    unsigned char byte;
    //Current value of the dma
    unsigned char value;
    //Needed delay of the dma
    unsigned char delay;
};

void dma_start(unsigned char start);
void dma_tick();

bool dma_transfer();
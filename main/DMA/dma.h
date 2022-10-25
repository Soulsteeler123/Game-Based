#pragma once

#include "../reusable.h"

struct dma_context {
    bool active;
    unsigned char byte;
    unsigned char value;
    unsigned char delay;
};

void dma_start(unsigned char start);
void dma_tick();

bool dma_transfer();
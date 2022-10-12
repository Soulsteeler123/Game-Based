#pragma once
#include "../reusable.h"

struct timer_context {
    unsigned short div;
    unsigned char tima;
    unsigned char tma;
    unsigned char tac;
};

void timer_init();
void timer_tick();

unsigned char timer_read(unsigned short address);
void timer_write(unsigned short address, unsigned char value);

timer_context *get_timer();
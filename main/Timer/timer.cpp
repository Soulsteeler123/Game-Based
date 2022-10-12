#include "timer.h"
#include "../CPU/cpu.h"

timer_context tim_context = {0};

void timer_init() {
    tim_context.div = 0xAC00;
}
void timer_tick() {
    unsigned short prev_div = tim_context.div;
    tim_context.div++;

    bool update = false;
    switch(tim_context.tac & (0b11)) {
        case 0b00: update = (prev_div & (1 << 9)) && (!(tim_context.div & (1 << 9))); break;
        case 0b01: update = (prev_div & (1 << 3)) && (!(tim_context.div & (1 << 3))); break;
        case 0b10: update = (prev_div & (1 << 5)) && (!(tim_context.div & (1 << 5))); break;
        case 0b11: update = (prev_div & (1 << 7)) && (!(tim_context.div & (1 << 7))); break;
    }

    if(update && tim_context.tac & (1 << 2)) {
        tim_context.tima++;
        if(tim_context.tima == 0xFF)
            tim_context.tima = tim_context.tma;

            request_interrupt(TIMER);
    }
}

unsigned char timer_read(unsigned short address) {
    switch(address) {
        case 0xFF04: return tim_context.div >> 8;
        case 0xFF05: return tim_context.tima;
        case 0xFF06: return tim_context.tma;
        case 0xFF07: return tim_context.tac;
        default: return 0;
    }
}
void timer_write(unsigned short address, unsigned char value) {
    switch(address) {
        case 0xFF04: tim_context.div = 0; break;
        case 0xFF05: tim_context.tima = value; break;
        case 0xFF06: tim_context.tma = value; break;
        case 0xFF07: tim_context.tac = value; break;
    }
}

timer_context *get_timer() {
    return &tim_context;
}
#pragma once
#include "reusable.h"

//Contains context regarding the emulator, if its paused, if its running, how many ticks (timer, cpu...) has passsed
typedef struct {
    bool paused;
    bool running;
    unsigned long long ticks;
} emu_struct;

//Main emulator function
int emu_main(int argc, char *argv[]);
//Returns address of the context structure
emu_struct *emu_get_struct();
//Increaments the cycles
void cycles(int cycle);
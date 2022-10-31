#pragma once
#include "reusable.h"

const int WIDTH = 1024;
const int HEIGHT = 768;

//Contains context regarding the emulator, if its paused, if its running, how many ticks (timer, cpu...) has passsed
typedef struct {
    bool paused;
    bool running;
    bool die;
    unsigned long long ticks;
} emu_struct;

//Main emulator function
int emu_main(int argc, char *argv[]);
//Returns address of the context structure
emu_struct *emu_get_struct();
//Increaments the cycles
void cycles(int cycle);

//Handles the SDL events
void sdl_events();

//Runs the cpu
void cpu_run();

void sdl_update();

unsigned long get_ticks();
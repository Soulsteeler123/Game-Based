#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDl_ttf.h>
#include "main.h"
#include "Cartridge/cart.h"
#include "CPU/cpu.h"

//Contains all emulator context
static emu_struct context;

//Initial function to be called
int main(int argc, char* argv[]){
    emu_main(argc, argv);
    return 0;
}

/*
    Will control all main components of the emulator (Cart, CPU, Bus, PPU, Timer)
*/
int emu_main(int argc, char* argv[]) {
    //Checks if rom file was passed when executable is run
    if(argc < 2) {
        std::cout << "Error, rom file not passed. Please re-run with path to rom file." << std::endl;
        return -1;
    }
    
    //Checks if the cartridge loaded correctly
    if(!cart_load(argv[1])) {
        std::cout << "Error, failed to load rom file. Please check if rom file is valid for gameboy emulation." << std::endl;
        return -1;
    }

    std::cout << "Rom Cartridge loaded..." << std::endl;

    //Initializes SDL methods
    SDL_Init(SDL_INIT_VIDEO);
    std::cout << "SDL Initialized..." << std::endl;
    TTF_Init();
    std::cout << "TTF Initialized..." << std::endl;

    //Initializes cpu
    cpu_init();

    //Auto sets values for the context
    context.running = true;
    context.paused = false;
    context.ticks = 0;

    //Loops while the rom is running
    while(context.running) {
        //Delays program if the context is paused
        if(context.paused) {
            delay(10);
            continue;
        }

        //Error in case cpu is not able to step
        if(!cpu_step()) {
            std::cout << "Error, CPU stopped..." << std::endl;
            return -1;
        }

        //Increaments cpu ticks for each cycle
        context.ticks++;
    }

    return 0;
}

emu_struct *emu_get_struct() {
    //Returns address of context
    return &context;
}

void delay(unsigned int ms) {
    //Will delay the program window
    SDL_Delay(ms);
}

void cycles(int cycle) {
    //Not yet implemented
}
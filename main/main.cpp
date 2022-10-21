#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDl_ttf.h>
#include <chrono>
#include "main.h"
#include "Cartridge/cart.h"
#include "CPU/cpu.h"
#include "Timer/timer.h"
//Contains all emulator context
static emu_struct context;

//All SDL values and information
SDL_Window *sdlWindow;
SDL_Renderer *sdlRender;
SDL_Texture *sdlText;
SDL_Surface *sdlSurf;

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
    std::remove("output.txt");
    //Creates game window
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &sdlWindow, &sdlRender);
    //Makes thread for CPU
    std::thread t1(cpu_run);
    //Runs until the game window closes
    while(!context.die) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        sdl_events();
    }
    return 0;
}

emu_struct *emu_get_struct() {
    //Returns address of context
    return &context;
}

void delay(unsigned long ms) {
    //Will delay the program window
    SDL_Delay(ms);
}

void cycles(int cycle) {
    
    int num = cycle * 4;

    for(int i = 0; i < num; i++) {
        //Increaments cpu ticks for each cycle
        context.ticks++;
        timer_tick();
    }
    
}

void sdl_events() {
    SDL_Event event;
    //Properly closes program if window is closed
    while(SDL_PollEvent(&event) > 0) {
        /**
         * SDL_UpdateWindowSurface(sdlWindow); 
         * SDL_UpdateWindowSurface(sdlTraceWindow); 
         * SDL_UpdateWindowSurface(sdlDebugWindow); 
         */
        if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
            emu_get_struct()->die = true;
    }
}

void cpu_run() {
    //Initializes cpu
    cpu_init();
    timer_init();
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
            NOT_IMPL
        }
    }
}
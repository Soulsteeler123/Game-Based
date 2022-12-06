#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDl_ttf.h>
#include <chrono>
#include "main.h"
#include "Cartridge/cart.h"
#include "CPU/cpu.h"
#include "Timer/timer.h"
#include "BUS/bus.h"
#include "DMA/dma.h"
#include "PPU/ppu.h"

//Contains all emulator context
static emu_struct context;

//All SDL values and information
SDL_Window *sdlWindow;
SDL_Renderer *sdlRender;
SDL_Texture *sdlText;
SDL_Surface *sdlSurf;

SDL_Window *sdlDebugWindow;
SDL_Renderer *sdlDebugRender;
SDL_Texture *sdlDebugText;
SDL_Surface *sdlDebugSurf;

static int scale = 4;
static unsigned long tile_colors[4] = {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};

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

    sdlSurf = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    sdlText = SDL_CreateTexture(sdlRender, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    SDL_CreateWindowAndRenderer(16 * 8 * scale, 32 * 8 * scale, 0, &sdlDebugWindow, &sdlDebugRender);
    sdlDebugSurf = SDL_CreateRGBSurface(0, (16 * 8 * scale) + (16 * scale), (32 * 8 * scale) + (64 * scale), 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    sdlDebugText = SDL_CreateTexture(sdlDebugRender, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, (16 * 8 * scale) + (16 * scale), (32 * 8 * scale) + (64 * scale));
    int x, y;
    SDL_GetWindowPosition(sdlWindow, &x, &y);
    SDL_SetWindowPosition(sdlDebugWindow, x + WIDTH + 10, y);

    //Makes thread for CPU
    std::thread t1(cpu_run);
    //Runs until the game window closes
    unsigned long prev_frame = 0;
    while(!context.die) {
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        sdl_events();
        if(prev_frame != ppu_get_context()->frame)
            sdl_update();
        prev_frame = ppu_get_context()->frame;
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

unsigned long get_ticks() {
    return SDL_GetTicks();
}

void cycles(int cycle) {
    for(int i = 0; i < cycle; i++) {
        for (int j = 0; j < 4; j++) {
            context.ticks++;
            timer_tick();
            ppu_tick();
        }
        dma_tick();
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
    //Initializes timer
    timer_init();
    //Initializes cpu
    cpu_init();
    //Initializes ppu
    ppu_init();
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

void display_tile(SDL_Surface *surface, unsigned short start, unsigned short tileNum, int x, int y) {
    SDL_Rect rc;
    unsigned char byte1, byte2, hi, lo, color;
    for(int tileY = 0; tileY < 16; tileY += 2) {
        //Gets tile bytes from bus
        byte1 = bus_read(start + (tileNum * 16) + tileY);
        byte2 = bus_read(start + (tileNum * 16) + tileY + 1);

        for(int bit = 7; bit >= 0; bit--) {
            //Gets lo and hi of tyle bytes
            hi = !!(byte1 & (1 << bit)) << 1;
            lo = !!(byte2 & (1 << bit));

            //Gets the color number (0 through 3)
            color = hi | lo;
            //Sets the correct tile values
            rc.x = x + ((7 - bit) * scale);
            rc.y = y + (tileY / 2 * scale);
            rc.w = scale;
            rc.h = scale;

            //Displays the tile
            SDL_FillRect(surface, &rc, tile_colors[color]);
        }
    }
}

void sdldebug_update() {
    int xDraw = 0, yDraw = 0, tileNum = 0;
    SDL_Rect rc;
    //Default address value
    unsigned short address = 0x8000;
    //Sets the tile values
    rc.x = 0;
    rc.y = 0;
    rc.w = sdlDebugSurf->w;
    rc.h = sdlDebugSurf->h;
    //Places the tile
    SDL_FillRect(sdlDebugSurf, &rc, 0xFF111111);

    for(int y = 0; y < 24; y++) {
        for(int x = 0; x < 16; x++) {
            display_tile(sdlDebugSurf, address, tileNum, xDraw + (x * scale), yDraw + (y * scale));
            xDraw += (8 * scale);
            tileNum++;
        }
        yDraw += (8 * scale);
        xDraw = 0;
    }

    SDL_UpdateTexture(sdlDebugText, NULL, sdlDebugSurf->pixels, sdlDebugSurf->pitch);
    SDL_RenderClear(sdlDebugRender);
    SDL_RenderCopy(sdlDebugRender, sdlDebugText, NULL, NULL);
    SDL_RenderPresent(sdlDebugRender);
}

void sdl_update() {
    SDL_Rect rc;
    //Initializes the window values
    rc.x = rc.y = 0;
    rc.w = rc.h = 2048;

    //Gets the buffer from ppu
    unsigned long *buffer = ppu_get_context()->video_buffer;
    
    for(int y = 0; y < YRES; y++) {
        for(int x = 0; x < XRES; x++) {
            //Gets the window values for current tile
            rc.x = x * scale;
            rc.y = y * scale;
            rc.w = scale;
            rc.h = scale;

            //Places tile on screen
            SDL_FillRect(sdlSurf, &rc, buffer[x + (y * XRES)]);
        }
    }

    //Updates the tile
    SDL_UpdateTexture(sdlText, NULL, sdlSurf->pixels, sdlSurf->pitch);
    //Clears renderer
    SDL_RenderClear(sdlRender);
    //Copies the texture to the renderer
    SDL_RenderCopy(sdlRender, sdlText, NULL, NULL);
    //Presents the renderer
    SDL_RenderPresent(sdlRender);

    sdldebug_update();
}
#pragma once
#include "../reusable.h"

//Constant values for ppu
const int LINES_PER_FRAME = 154;
const int TICKS_PER_LINE = 456;
const int YRES = 144;
const int XRES = 160;

//Current state of PPU
enum state {
    TILE,
    DATA0,
    DATA1,
    IDLE,
    PUSH
};

//Details the fifo entry
struct fifo_entry {
    //Next entry in the fifo
    fifo_entry *next;
    //32 bit color value
    unsigned long value;
};

//Defines the pixel fifo
struct fifo {
    fifo_entry *head;
    fifo_entry *tail;
    unsigned long size;
};

struct fifo_context {
    //Current ppu state
    state cur_state;
    //current fifo pixel
    fifo pixel_fifo;
    //Current line on the x
    unsigned char line_x;
    //Current X value pushed on ppu
    unsigned char pushed_x;
    //Current fetched x value
    unsigned char fetch_x;
    //Background window data
    unsigned char bgw_data[3];
    //OAM data
    unsigned char entry_data[6];
    //Map of y tile
    unsigned char map_y;
    //Map of X tile
    unsigned char map_x;
    //Y tile
    unsigned char tile_y;
    //Fifo size
    unsigned char fifo_x;
};

//Details PPU OAM
struct oam {
    unsigned char y;
    unsigned char x;
    unsigned char tile;
    //Colored background panel
    unsigned char cgb_pn : 3;
    unsigned char vram : 1;
    unsigned char pn : 1;
    unsigned char xflip : 1;
    unsigned char yflip : 1;
    //Background color palette
    unsigned char bgp : 1;
};

struct ppu_context {
    //OAM ram data
    oam oam_ram[40];
    //VRAM data
    unsigned char vram[0x2000];
    //Pixel fifo color
    fifo_context pfc;
    //Current frame
    unsigned long frame;
    //Current number of line ticks
    unsigned long line_ticks;
    unsigned long *video_buffer;
};
void ppu_init();
void ppu_tick();

void ppu_oam_write(unsigned short address, unsigned char value);
unsigned char ppu_oam_read(unsigned short address);

void ppu_vram_write(unsigned short address, unsigned char value);
unsigned char ppu_vram_read(unsigned short address);

ppu_context *ppu_get_context();

void pipeline_fifo_reset();
void pipeline_process();
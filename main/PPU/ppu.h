#pragma once
#include "../reusable.h"

const int LINES_PER_FRAME = 154;
const int TICKS_PER_LINE = 456;
const int YRES = 144;
const int XRES = 160;

struct oam {
    unsigned char y;
    unsigned char x;
    unsigned char tile;
    unsigned cgb_pn : 3;
    unsigned vram : 1;
    unsigned pn : 1;
    unsigned xflip : 1;
    unsigned yflip : 1;
    unsigned bgp : 1;
};

struct ppu_context {
    oam oam_ram[40];
    unsigned char vram[0x2000];

    unsigned long frame;
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
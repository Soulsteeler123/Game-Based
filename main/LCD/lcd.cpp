#include "lcd.h"
#include "../PPU/ppu.h"
#include "../DMA/dma.h"

static lcd_context context;
static unsigned long default_colors[4] = {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};

void update_palette(unsigned char data, unsigned char palette) {
    unsigned long *p_colors = context.bg_colors;

    //Determines which sprite colors to use based on palette number
    switch(palette) {
        case 1: p_colors = context.sp1_colors; break;
        case 2: p_colors = context.sp2_colors; break;
    }

    //Sets the palette colors
    p_colors[0] = default_colors[data & 0b11];
    p_colors[1] = default_colors[(data >> 2) & 0b11];
    p_colors[2] = default_colors[(data >> 4) & 0b11];
    p_colors[3] = default_colors[(data >> 6) & 0b11];
}

void lcd_init() {
    //Initializes default lcd values
    context.lcdc = 0x91;
    context.scroll_x = 0;
    context.scroll_y = 0;
    context.ly = 0;
    context.lyc = 0;
    context.bg_palette = 0xFC;
    context.obj_palette[0] = 0xFF;
    context.obj_palette[1] = 0xFF;
    context.win_y = 0;
    context.win_x = 0;

    for(int i = 0; i < 4; i++) {
        context.bg_colors[i] = default_colors[i];
        context.sp1_colors[i] = default_colors[i];
        context.sp2_colors[i] = default_colors[i];
    }
}

lcd_context *lcd_get_context() {
    return &context;
}

unsigned char lcd_read(unsigned short address) {
    //Determines the offset
    unsigned char offset = (address - 0xFF40);
    //Returns the value of the context based on the offset
    unsigned char *pointer = (unsigned char *)&context;
    return pointer[offset];
}

void lcd_write(unsigned short address, unsigned char value) {
    unsigned char offset = (address - 0xFF40);
    unsigned char *pointer = (unsigned char *)&context;
    pointer[offset] = value;

    //DMA
    if(offset == 6) 
        dma_start(value);
    
    //Updates the palette based on which address is passed in
    if(address == 0xFF47)
        update_palette(value, 0);
    else if(address == 0xFF48)
        update_palette(value & 0b11111100, 1);
    else if(address == 0xFF49)
        update_palette(value & 0b11111100, 2);
    
}
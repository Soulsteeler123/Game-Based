#include "ppu.h"
#include "../LCD/lcd.h"
#include "sm.h"

static ppu_context context;

ppu_context *ppu_get_context() {
    return &context;
}

void ppu_init() {
    //Sets default values of PPU
    context.frame = 0;
    context.line_ticks = 0;
    context.video_buffer = new unsigned long[YRES * XRES * sizeof(32)];

    context.pfc.line_x = 0;
    context.pfc.pushed_x = 0;
    context.pfc.fetch_x = 0;
    context.pfc.pixel_fifo.size = 0;
    context.pfc.pixel_fifo.head = context.pfc.pixel_fifo.tail = NULL;
    context.pfc.cur_state = TILE;
    //Initializes the LCD
    lcd_init();
    LCDS_MODE_SET(MODE_OAM);

    //Sets the memory for the PPU
    memset(context.oam_ram, 0, sizeof(context.oam_ram));
    memset(context.video_buffer, 0, YRES * XRES * sizeof(unsigned long));
}
void ppu_tick() {
    //Increments line ticks
    context.line_ticks++;

    //Calls appropriate PPU mode
    switch(LCDS_MODE) {
        case MODE_OAM: ppu_mode_oam(); break;
        case MODE_XFER: ppu_mode_xfer(); break;
        case MODE_VBLANK: ppu_mode_vblank(); break;
        case MODE_HBLANK: ppu_mode_hblank(); break;
    }
}

void ppu_oam_write(unsigned short address, unsigned char value) {
    //Sets address to correct address if needed
    if(address >= 0xFE00)
        address -= 0xFE00;
    
    //Sets oam at address to value
    unsigned char *pointer = (unsigned char *)context.oam_ram;
    pointer[address] = value;
}

unsigned char ppu_oam_read(unsigned short address) {
    //Sets address to correct address if needed
    if(address >= 0xFE00)
        address -= 0xFE00;
    
    //Returns value at oam address
    unsigned char *pointer = (unsigned char *)context.oam_ram;
    return pointer[address];
}

void ppu_vram_write(unsigned short address, unsigned char value) {
    //Sets VRAM at address to value
    context.vram[address - 0x8000] = value;
}

unsigned char ppu_vram_read(unsigned short address) {
    //Returns value at VRAM address
    return context.vram[address - 0x8000];
}
#include "ppu.h"
#include "../LCD/lcd.h"
#include "sm.h"

static ppu_context context;

ppu_context *ppu_get_context() {
    return &context;
}

void ppu_init() {
    context.frame = 0;
    context.line_ticks = 0;
    context.video_buffer = (unsigned long *)(malloc(YRES * XRES * sizeof(32)));
    lcd_init();
    LCDS_MODE_SET(MODE_OAM);

    memset(context.oam_ram, 0, sizeof(context.oam_ram));
    memset(context.video_buffer, 0, YRES * XRES * sizeof(unsigned long));
}
void ppu_tick() {  
    context.line_ticks++;

    switch(LCDS_MODE) {
        case MODE_OAM: ppu_mode_oam(); break;
        case MODE_XFER: ppu_mode_xfer(); break;
        case MODE_VBLANK: ppu_mode_vblank(); break;
        case MODE_HBLANK: ppu_mode_hblank(); break;
    }
}

void ppu_oam_write(unsigned short address, unsigned char value) {
    if(address >= 0xFE00)
        address -= 0xFE00;
    
    unsigned char *pointer = (unsigned char *)context.oam_ram;
    pointer[address] = value;
}

unsigned char ppu_oam_read(unsigned short address) {
    if(address >= 0xFE00)
        address -= 0xFE00;
    
    unsigned char *pointer = (unsigned char *)context.oam_ram;
    return pointer[address];
}

void ppu_vram_write(unsigned short address, unsigned char value) {
    context.vram[address - 0x8000] = value;
}

unsigned char ppu_vram_read(unsigned short address) {
    return context.vram[address - 0x8000];
}
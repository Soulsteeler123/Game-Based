#pragma once

#include "../reusable.h"

struct lcd_context {
    //registers
    unsigned char lcdc;
    unsigned char lcds;
    unsigned char scroll_y;
    unsigned char scroll_x;
    unsigned char ly;
    unsigned char lyc;
    unsigned char dma;
    unsigned char bg_palette;
    unsigned char obj_palette[2];
    unsigned char win_y;
    unsigned char win_x;

    unsigned long bg_colors[4];
    unsigned long sp1_colors[4];
    unsigned long sp2_colors[4];
};

enum lcd_mode {
    MODE_HBLANK,
    MODE_VBLANK,
    MODE_OAM,
    MODE_XFER
};

enum stat_src {
    SS_HBLANK = (1 << 3),
    SS_VBLANK = (1 << 4),
    SS_OAM = (1 << 5),
    SS_LYC = (1 << 6)
};

lcd_context *lcd_get_context();
void lcd_init();

unsigned char lcd_read(unsigned short address);
void lcd_write(unsigned short address, unsigned char value);
//LCD control
#define LCDC_BGW_ENABLE (BIT(lcd_get_context()->lcdc, 0))
#define LCDC_OBJ_ENABLE (BIT(lcd_get_context()->lcdc, 1))
#define LCDC_OBJ_HEIGHT (BIT(lcd_get_context()->lcdc, 2) ? 16 : 8)
#define LCDC_BG_MAP_AREA (BIT(lcd_get_context()->lcdc, 3) ? 0x9C00 : 0x9800)
#define LCDC_BGW_DATA_AREA (BIT(lcd_get_context()->lcdc, 4) ? 0x8000 : 0x8800)
#define LCDC_WIN_ENABLE (BIT(lcd_get_context()->lcdc, 5))
#define LCDC_WIN_MAP_AREA (BIT(lcd_get_context()->lcdc, 6) ? 0x9C00 : 0x9800)
#define LCDC_LCD_ENABLE (BIT(lcd_get_context()->lcdc, 7))

//LCD Status
#define LCDS_MODE ((lcd_mode)(lcd_get_context()->lcds & 0b11))
#define LCDS_MODE_SET(mode) { lcd_get_context()->lcds &= ~0b11; lcd_get_context()->lcds |= mode; }

//LYC flag
#define LCDS_LYC (BIT(lcd_get_context()->lcds, 2))
#define LCDS_LYC_SET(b) { BIT_SET(lcd_get_context()->lcds, 2, b); }

//Interrupt source
#define LCDS_STAT_INT(src) (lcd_get_context()->lcds & src)
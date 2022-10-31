#include "sm.h"
#include "ppu.h"
#include "../LCD/lcd.h"
#include "../CPU/cpu.h"

unsigned long target_frame = 1000/60;
long prev_frame = 0;
long start_timer = 0;
long frame_count = 0;

void increment_ly() {
    lcd_get_context()->ly++;

    if(lcd_get_context()->ly == lcd_get_context()->lyc) {
        LCDS_LYC_SET(1);

        if(LCDS_STAT_INT(SS_LYC))
            request_interrupt(LCD_STAT);
    }
    else
        LCDS_LYC_SET(0);
}

void ppu_mode_oam() {
    if(ppu_get_context()->line_ticks >= 80)
        LCDS_MODE_SET(MODE_XFER)
}

void ppu_mode_xfer() {
    if(ppu_get_context()->line_ticks >= 252)
        LCDS_MODE_SET(MODE_HBLANK)
}

void ppu_mode_vblank() {
    if(ppu_get_context()->line_ticks >= TICKS_PER_LINE) {
        increment_ly();

        if(lcd_get_context()->ly >= LINES_PER_FRAME) {
            LCDS_MODE_SET(MODE_OAM)
            lcd_get_context()->ly = 0;
        }

        ppu_get_context()->line_ticks = 0;
        
    }
}



void ppu_mode_hblank() {
    if(ppu_get_context()->line_ticks >= TICKS_PER_LINE) {
        increment_ly();

        if(lcd_get_context()->ly >= YRES) {
            LCDS_MODE_SET(MODE_VBLANK)
            request_interrupt(VBLANK);

            if(LCDS_STAT_INT(SS_VBLANK))
                request_interrupt(LCD_STAT);

            ppu_get_context()->frame++;

            //Calculate FPS
            unsigned long end = get_ticks();
            unsigned long time_frame = end - prev_frame;

            if(time_frame < target_frame) 
                delay((target_frame - time_frame));
            
            if(end - start_timer >= 1000) {
                unsigned long fps = frame_count;
                start_timer = end;
                frame_count = 0;
                std::cout << "FPS: " << fps << std::endl;
            }
            
            frame_count++;
            prev_frame = get_ticks();

        }
        else {
            LCDS_MODE_SET(MODE_OAM)
        }

        ppu_get_context()->line_ticks = 0;
    }
}
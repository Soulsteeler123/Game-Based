#include "sm.h"
#include "ppu.h"
#include "../LCD/lcd.h"
#include "../CPU/cpu.h"

//Default values for fps calculation
unsigned long target_frame = 1000/60;
long prev_frame = 0;
long start_timer = 0;
long frame_count = 0;

void increment_ly() {
    //Increments ly by 1
    lcd_get_context()->ly++;

    //If ly is equal to ly compare
    if(lcd_get_context()->ly == lcd_get_context()->lyc) {
        LCDS_LYC_SET(1);

        if(LCDS_STAT_INT(SS_LYC))
            request_interrupt(LCD_STAT);
    }
    else
        LCDS_LYC_SET(0);
}

//Sets values for the pfc
void ppu_mode_oam() {
    //If there is more than or equal to 80 ticks
    if(ppu_get_context()->line_ticks >= 80) {
        LCDS_MODE_SET(MODE_XFER)

        ppu_get_context()->pfc.cur_state = TILE;
        ppu_get_context()->pfc.line_x = 0;
        ppu_get_context()->pfc.fetch_x = 0;
        ppu_get_context()->pfc.pushed_x = 0;
        ppu_get_context()->pfc.fifo_x = 0;
    }
}

void ppu_mode_xfer() {
    //Calls for the pipeline to be processed
    pipeline_process();

    //Checks x value based on the X resolution
    if(ppu_get_context()->pfc.pushed_x >= XRES) {
        //Resets pipeline fifo and sets the LCD state mode to HBLANK
        pipeline_fifo_reset();
        LCDS_MODE_SET(MODE_HBLANK);

        //Request interrupt if needed
        if(LCDS_STAT_INT(SS_HBLANK))
            request_interrupt(LCD_STAT);
    }
}

void ppu_mode_vblank() {
    //If line ticks exceed the ticks per line
    if(ppu_get_context()->line_ticks >= TICKS_PER_LINE) {
        //Increments ly
        increment_ly();

        //If ly exceeds the lines per frame
        if(lcd_get_context()->ly >= LINES_PER_FRAME) {
            //Sets the LCD state mode to OAM
            LCDS_MODE_SET(MODE_OAM);
            //Resets ly
            lcd_get_context()->ly = 0;
        }  

        //Resets line ticks
        ppu_get_context()->line_ticks = 0;
    }
}

void ppu_mode_hblank() {
    //If line ticks exceed ticks per line
    if(ppu_get_context()->line_ticks >= TICKS_PER_LINE) {
        //Increments ly
        increment_ly();

        //If ly exceeds the Y resolution
        if(lcd_get_context()->ly >= YRES) {
            //Resets LCD state mode to VBLANK
            LCDS_MODE_SET(MODE_VBLANK);
            //Requests interrupt
            request_interrupt(VBLANK);

            //If the state interrupt is VBLANK then request interrupt
            if(LCDS_STAT_INT(SS_VBLANK))
                request_interrupt(LCD_STAT);
            
            //Increments frame
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
        //Else just sets LCD state mode to OAM
        else {
            LCDS_MODE_SET(MODE_OAM)
        }

        //Resets line ticks
        ppu_get_context()->line_ticks = 0;
    }
}
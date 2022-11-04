#include "ppu.h"
#include "../LCD/lcd.h"
#include "../BUS/bus.h"

void pixel_fifo_push(unsigned long value) {
    //Sets the next fifo entry
    fifo_entry *next = new fifo_entry[sizeof(fifo_entry)];
    //Sets next's default values
    next->next = NULL;
    next->value = value;

    if(!ppu_get_context()->pfc.pixel_fifo.head) {
        //first entry
        ppu_get_context()->pfc.pixel_fifo.head = ppu_get_context()->pfc.pixel_fifo.tail = next;
    }
    //Else appends to end of fifo
    else {
        ppu_get_context()->pfc.pixel_fifo.tail->next = next;
        ppu_get_context()->pfc.pixel_fifo.tail = next;
    }

    //Increments size
    ppu_get_context()->pfc.pixel_fifo.size++;
}

unsigned long pixel_fifo_pop() {
    //If attempting to pop without anything in fifo
    if(ppu_get_context()->pfc.pixel_fifo.size <= 0) {
        std::cout << "ERROR IN PIXEL FIFO. INVALID POP" << std::endl;
        NOT_IMPL
    }
    else {
        //Removes head and makes next value the head
        fifo_entry *popped = ppu_get_context()->pfc.pixel_fifo.head;
        ppu_get_context()->pfc.pixel_fifo.head = popped->next;
        ppu_get_context()->pfc.pixel_fifo.size--;

        //Frees the popped node
        unsigned long value = popped->value;
        free(popped);

        //Returns the old head
        return value;
    }
}

bool pipeline_fifo_add() {
    if(ppu_get_context()->pfc.pixel_fifo.size > 8) 
        //fifo is full
        return false;
        //Gets the x from the pfc
    int x = ppu_get_context()->pfc.fetch_x - (8 - (lcd_get_context()->scroll_x % 8));

    int bit;
    unsigned char hi, lo;
    unsigned long color;
    for(int i = 0; i < 8; i++) {
        //Gets the bit, hi, lo and color from the pfc
        bit = 7 - i;
        lo = !!(ppu_get_context()->pfc.bgw_data[1] & (1 << bit));
        hi = !!(ppu_get_context()->pfc.bgw_data[2] & (1 << bit)) << 1;
        color = lcd_get_context()->bg_colors[lo | hi];

        //Assuming x is correct then pushes the color onto the fifo
        if(x >= 0) {
            pixel_fifo_push(color);
            ppu_get_context()->pfc.fifo_x++;
        }
    }
    
    //Successfully added color to fifo
    return true;
}

void pipeline_fetch() {
    //Assigns the correct background window data based on the BUS and moves to the next state mode
    switch(ppu_get_context()->pfc.cur_state) {
        case TILE: {
            if(LCDC_BGW_ENABLE) {
                ppu_get_context()->pfc.bgw_data[0] = bus_read(LCDC_BG_MAP_AREA + (ppu_get_context()->pfc.map_x / 8) + ((ppu_get_context()->pfc.map_y / 8) * 32));

                if(LCDC_BGW_DATA_AREA == 0x8800) 
                    ppu_get_context()->pfc.bgw_data[0] += 128;
                
            }

            ppu_get_context()->pfc.cur_state = DATA0;
            ppu_get_context()->pfc.fetch_x += 8;
        } break;

        case DATA0: {
            ppu_get_context()->pfc.bgw_data[1] = bus_read(LCDC_BGW_DATA_AREA + (ppu_get_context()->pfc.bgw_data[0] * 16) + ppu_get_context()->pfc.tile_y);

            ppu_get_context()->pfc.cur_state = DATA1;
        } break;

        case DATA1: {
            ppu_get_context()->pfc.bgw_data[2] = bus_read(LCDC_BGW_DATA_AREA + (ppu_get_context()->pfc.bgw_data[0] * 16) + ppu_get_context()->pfc.tile_y + 1);

            ppu_get_context()->pfc.cur_state = IDLE;
        } break;

        case IDLE: {
            ppu_get_context()->pfc.cur_state = PUSH;
        } break;

        case PUSH: {
            if(pipeline_fifo_add()) 
                ppu_get_context()->pfc.cur_state = TILE;
        } break;
    }
}

void pipleine_push_pixel() {
    //If fifo isn't full
    if(ppu_get_context()->pfc.pixel_fifo.size > 8) {
        //Pops the head of the pixel fifo
        unsigned long data = pixel_fifo_pop();

        //If the lines have exceeded then assigns the video puffer to the head of the fifo
        if(ppu_get_context()->pfc.line_x >= (lcd_get_context()->scroll_x % 8)) {
            ppu_get_context()->video_buffer[ppu_get_context()->pfc.pushed_x + (lcd_get_context()->ly * XRES)] = data;
            ppu_get_context()->pfc.pushed_x++;
        }

        //Increments the line number
        ppu_get_context()->pfc.line_x++;
    }
}

void pipeline_process() {
    //Gets the maps and y tile
    ppu_get_context()->pfc.map_y = (lcd_get_context()->ly + lcd_get_context()->scroll_y);
    ppu_get_context()->pfc.map_x = (ppu_get_context()->pfc.fetch_x + lcd_get_context()->scroll_x);
    ppu_get_context()->pfc.tile_y = ((lcd_get_context()->ly + lcd_get_context()->scroll_y) % 8) * 2;

    //If the line ticks are odd then fetch the pipeline
    if(!(ppu_get_context()->line_ticks & 1)) 
        pipeline_fetch();

    //Push the pixel
    pipleine_push_pixel();
}

void pipeline_fifo_reset() {
    //Removes all nodes from fifo
    while(ppu_get_context()->pfc.pixel_fifo.size) {
        pixel_fifo_pop();
    }

    //Sets the head to an empty space
    ppu_get_context()->pfc.pixel_fifo.head = 0;
}
#include "bus.h"
#include "../Cartridge/cart.h"
#include "../RAM/ram.h"
#include "../CPU/cpu.h"
#include "../Timer/timer.h"
#include "../PPU/ppu.h"
#include "../DMA/dma.h"
#include "../LCD/lcd.h"
char data[2];
unsigned char bus_read(unsigned short address) {
    //Cartridge address
    if(address < 0x8000)
        return cart_read(address);
    //Map Data
    else if(address < 0xA000) {
        return ppu_vram_read(address);
    }
    //Cartridge address
    else if(address < 0xC000) {
        return cart_read(address);
    }
    //Working ram address
    else if(address < 0xE000) {
        return wram_read(address);
    }
    else if(address < 0xFE00) {
        //Echo ram, not needed
        return 0;
    }
    else if(address < 0xFEA0) {
        //0AM
        if ((dma_transfer()))
            return 0xFF;
        else
            return ppu_oam_read(address);
    }
    else if(address < 0xFF00) {
        //Unusable
        return 0;
    }
    else if(address < 0xFF80) {
        //TODO
        return io_read(address);
    }
    //Indicates need for ie register
    else if (address == 0xFFFF) {
        return get_ie_reg();
    }
    //High ram address
    else {
        return hram_read(address);
    }
}

void bus_write(unsigned short address, unsigned char value) {
    //Cartridge address
    if(address < 0x8000) 
        cart_write(address, value);
    else if(address < 0xA000) {
        ppu_vram_write(address, value);
    }
    //Cartridge address
    else if(address < 0xC000) {
       cart_write(address, value);
    }
    //Working ram address
    else if(address < 0xE000) {
        wram_write(address, value);
    }
    else if(address < 0xFE00) {
        //Echo ram, not needed
    }
    else if(address < 0xFEA0) {
        if(!dma_transfer())
            ppu_oam_write(address, value);
    }
    else if(address < 0xFF00) {
        //Unusable
    }
    else if(address < 0xFF80) {
        //TODO
        io_write(address, value);
    }
    //Indicates need to set ie register to value
    else if (address == 0xFFFF) {
        set_ie_reg(value);
    }
    //High ram address
    else {
        hram_write(address, value);
    }
}

unsigned short bus_read16(unsigned short address) {
    //Gets low and high bits of a given 16 bit address
    unsigned short lo = bus_read(address);
    unsigned short hi = bus_read(address + 1);
    //Or's low and high bits with high bits shifted over to the left by 8 bits
    return lo | (hi << 8);
}

void bus_write16(unsigned short address, unsigned short value) {
    //Writes the 16 bit address with value shifted over to the left by 8 bits and with only the first two bits (low bits)
    bus_write(address + 1, (value >> 8) & 0xFF);
    //Writes the low bits of the 16 bit address with only first two bits (high bits)
    bus_write(address, value & 0xFF);
}

unsigned char io_read(unsigned short address) {
    //Returns first value of stored data
    if(address == 0xFF01)
        return data[0];
    //Returns second value of stored data
    else if(address == 0xFF02)
        return data[1];
    //Returns what's written to the timer
    else if(BETWEEN(address, 0xFF04, 0xFF07)) 
        return timer_read(address);
    //Returns the interrupt flags
    else if(address == 0xFF0F)
        return get_iflags();
    else if(BETWEEN(address, 0xFF40, 0xFF4B)) 
        return lcd_read(address);
    
    std::cout << "Unsupported bus read3 at " << std::setfill('0') << std::setw(4) << std::hex << (int)address << std::endl;
    return 0;
}
void io_write(unsigned short address, unsigned char value) {
    //Assigns address to the first half of data
    if(address == 0xFF01)
        data[0] = value;
    //Assigns address to the second half of data
    else if(address == 0xFF02)
        data[1] = value;
    //Writes value at the address to the timer
    else if(BETWEEN(address, 0xFF04, 0xFF07))
        timer_write(address, value);
    //Sets the interrupt flags to the given value
    else if(address == 0xFF0F)
        set_iflags(value);
    else if(BETWEEN(address, 0xFF40, 0xFF4B))
        lcd_write(address, value);
    else
        std::cout << "Unsupported bus write3 at " << std::setfill('0') << std::setw(4) << std::hex << (int)address << std::endl;
}
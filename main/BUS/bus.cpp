#include "bus.h"
#include "../Cartridge/cart.h"
#include "../RAM/ram.h"
#include "../CPU/cpu.h"

unsigned char bus_read(unsigned short address) {
    if(address < 0x8000)
        return cart_read(address);
    else if(address < 0xA000) {
        //TODO
        std::cout << "Unsupported bus read at " << std::setfill('0') << std::setw(4) << std::hex << (int)address << std::endl;
        NOT_IMPL
    }
    else if(address < 0xC000) {
        return cart_read(address);
    }
    else if(address < 0xE000) {
        return wram_read(address);
    }
    else if(address < 0xFE00) {
        //Echo ram, not needed
        return 0;
    }
    else if(address < 0xFEA0) {
        //0AM
        //TODO
        std::cout << "Unsupported bus read at " << std::setfill('0') << std::setw(4) << std::hex << (int)address << std::endl;
        NOT_IMPL
    }
    else if(address < 0xFF00) {
        //Unsuable
        return 0;
    }
    else if(address < 0xFF80) {
        //TODO
        std::cout << "Unsupported bus read at " << std::setfill('0') << std::setw(4) << std::hex << (int)address << std::endl;
        NOT_IMPL
    }
    else if (address == 0xFFFF) {
        return get_ie_reg();
    }
    else {
        return hram_read(address);
    }
}

void bus_write(unsigned short address, unsigned char value) {
    if(address < 0x8000) 
        cart_write(address, value);
    else if(address < 0xA000) {
        //TODO
        std::cout << "Unsupported bus read at " << std::setfill('0') << std::setw(4) << std::hex << (int)address << std::endl;
        NOT_IMPL
    }
    else if(address < 0xC000) {
        return cart_write(address, value);
    }
    else if(address < 0xE000) {
        return wram_write(address, value);
    }
    else if(address < 0xFE00) {
        //Echo ram, not needed
    }
    else if(address < 0xFEA0) {
        //0AM
        //TODO
        std::cout << "Unsupported bus read at " << std::setfill('0') << std::setw(4) << std::hex << (int)address << std::endl;
        NOT_IMPL
    }
    else if(address < 0xFF00) {
        //Unsuable
    }
    else if(address < 0xFF80) {
        //TODO
        std::cout << "Unsupported bus read at " << std::setfill('0') << std::setw(4) << std::hex << (int)address << std::endl;
        //NOT_IMPL
    }
    else if (address == 0xFFFF) {
        set_ie_reg(value);
    }
    else {
        return hram_write(address, value);
    }
}

unsigned short bus_read16(unsigned short address) {
    unsigned short lo = bus_read(address);
    unsigned short hi = bus_read(address + 1);
    return lo | (hi << 8);
}

void bus_write16(unsigned short address, unsigned short value) {
    bus_write(address + 1, (value >> 8) & 0xFF);
    bus_write(address, value & 0xFF);
}
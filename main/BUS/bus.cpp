#include "bus.h"
#include "../Cartridge/cart.h"

unsigned char bus_read(unsigned short address) {
    if(address < 0x8000)
        return cart_read(address);
    std::cout << "Unsupported bus read at " << std::setfill('0') << std::setw(4) << std::hex << (int)address << std::endl;
    return 0;
    //NOT_IMPL
}

void bus_write(unsigned short address, unsigned char value) {
    if(address < 0x8000) 
        cart_write(address, value);

    std::cout << "Unsupported bus write at " << std::setfill('0') << std::setw(4) << std::hex << (int)address << std::endl;
    //NOT_IMPL
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
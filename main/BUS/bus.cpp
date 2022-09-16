#include "bus.h"
#include "../Cartridge/cart.h"
unsigned char bus_read(unsigned short address) {
    if(address < 0x8000)
        return cart_read(address);
    NOT_IMPL
}

void bus_write(unsigned short address, unsigned char value) {
    if(address < 0x8000) 
        cart_write(address, value);
    else
        NOT_IMPL
}
#include "debug.h"
#include "../BUS/bus.h"
char message[1024] = {0};
int size = 0;
void debug_update() {
    if(bus_read(0xFF02) == 0x81) {
        message[size++] += bus_read(0xFF01);
        bus_write(0xFF02, 0);
    }
}

void debug_print() {
    if(message[0]) {
        std::cout << "Debug: " << message << std::endl;
    }
}
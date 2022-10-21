#include "debug.h"
#include "../BUS/bus.h"
//Message to output
char message[1024] = {0};
//Size of message
int size = 0;
void debug_update() {
    //If the right value was written to IO
    if(bus_read(0xFF02) == 0x81) {
        //Gets value of message then clears the IO
        message[size++] += bus_read(0xFF01);
        bus_write(0xFF02, 0);
    }
}

void debug_print() {
    //Outputs message if there is one
    if(message[0]) {
        std::cout << "Debug: " << message << std::endl;
    }
}
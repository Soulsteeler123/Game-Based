#pragma once
#include "../reusable.h"

struct rom_header{
    //4 byte entry memory
    unsigned char entry[4];

    //48 byte logo memory
    unsigned char logo[0x30];

    //Title of game
    char title[16]; 

    //New licensing code (publisher)
    unsigned short new_lic_code;

    //Determines if game supports SGB functions
    unsigned char sgb_flag;

    //Indicates type of hardware present on the cartridge
    unsigned char type;

    //Amount of rom on cartridge
    unsigned char rom_size;

    //Amount of ram on cartridge
    unsigned char ram_size;

    //Determines version of the game (Japan or overseas)
    unsigned char dest_code;

    //Old licensing code (publisher)
    unsigned char lic_code;

    //Version number of game
    unsigned char version;
    
    //Checksum implemented in cartridge
    unsigned char checksum;

    //Sum of all bytes of the cartridge rom
    unsigned short global_checksum;
};

//Loads cartridge
bool cart_load (std::string cart);

//Reads from the cartridge
unsigned char cart_read(unsigned short address);
//Writes to the carridge
void cart_write(unsigned short address, unsigned char value);
#pragma once
#include "reusable.h"

typedef struct {
    //4 byte entry memory
    uint8_t entry[4];

    //48 byte logo memory
    uint8_t logo[0x30];

    //Title of game
    char title[16]; 

    //New licensing code (publisher)
    uint16_t new_lic_code;

    //Determines if game supports SGB functions
    uint8_t sgb_flag;

    //Indicates type of hardware present on the cartridge
    uint8_t type;

    //Amount of rom on cartridge
    uint8_t rom_size;

    //Amount of ram on cartridge
    uint8_t ram_size;

    //Determines version of the game (Japan or overseas)
    uint8_t dest_code;

    //Old licensing code (publisher)
    uint8_t lic_code;

    //Version number of game
    uint8_t version;
    
    //Checksum implemented in cartridge
    uint8_t checksum;

    //Sum of all bytes of the cartridge rom
    uint16_t global_checksum;
} rom_header;

//Loads cartridge
bool cart_load (std::string cart);
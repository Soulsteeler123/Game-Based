#include "cart.h"

typedef struct {
    std::string filename;
    long rom_size;
    char* rom_data;
    rom_header *header;
} cart_struct;

static cart_struct context;

static std::string rom_types[] = {
    "ROM ONLY",
    "MBC1",
    "MBC1+RAM",
    "MBC1+RAM+BATTERY",
    "0x04 ???",
    "MBC2",
    "MBC2+BATTERY",
    "0x07 ???",
    "ROM+RAM 1",
    "ROM+RAM+BATTERY 1",
    "0x0A ???",
    "MMM01",
    "MMM01+RAM",
    "MMM01+RAM+BATTERY",
    "0x0E ???",
    "MBC3+TIMER+BATTERY",
    "MBC3+TIMER+RAM+BATTERY 2",
    "MBC3",
    "MBC3+RAM 2",
    "MBC3+RAM+BATTERY 2",
    "0x14 ???",
    "0x15 ???",
    "0x16 ???",
    "0x17 ???",
    "0x18 ???",
    "MBC5",
    "MBC5+RAM",
    "MBC5+RAM+BATTERY",
    "MBC5+RUMBLE",
    "MBC5+RUMBLE+RAM",
    "MBC5+RUMBLE+RAM+BATTERY",
    "0x1F ???",
    "MBC6",
    "0x21 ???",
    "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
};

static const std::map<int, std::string> lic_codes {
    {0x00, "None"},
    {0x01, "Nintendo R&D1"},
    {0x08, "Capcom"},
    {0x13, "Electronic Arts"},
    {0x18, "Hudson Soft"},
    {0x19, "b-ai"},
    {0x20, "kss"},
    {0x22, "pow"},
    {0x24, "PCM Complete"},
    {0x25, "san-x"},
    {0x28, "Kemco Japan"},
    {0x29, "seta"},
    {0x30, "Viacom"},
    {0x31, "Nintendo"},
    {0x32, "Bandai"},
    {0x33, "Ocean/Acclaim"},
    {0x34, "Konami"},
    {0x35, "Hector"},
    {0x37, "Taito"},
    {0x38, "Hudson"},
    {0x39, "Banpresto"},
    {0x41, "Ubi Soft"},
    {0x42, "Atlus"},
    {0x44, "Malibu"},
    {0x46, "angel"},
    {0x47, "Bullet-Proof"},
    {0x49, "irem"},
    {0x50, "Absolute"},
    {0x51, "Acclaim"},
    {0x52, "Activision"},
    {0x53, "American sammy"},
    {0x54, "Konami"},
    {0x55, "Hi tech entertainment"},
    {0x56, "LJN"},
    {0x57, "Matchbox"},
    {0x58, "Mattel"},
    {0x59, "Milton Bradley"},
    {0x60, "Titus"},
    {0x61, "Virgin"},
    {0x64, "LucasArts"},
    {0x67, "Ocean"},
    {0x69, "Electronic Arts"},
    {0x70, "Infogrames"},
    {0x71, "Interplay"},
    {0x72, "Broderbund"},
    {0x73, "sculptured"},
    {0x75, "sci"},
    {0x78, "THQ"},
    {0x79, "Accolade"},
    {0x80, "misawa"},
    {0x83, "lozc"},
    {0x86, "Tokuma Shoten Intermedia"},
    {0x87, "Tsukuda Original"},
    {0x91, "Chunsoft"},
    {0x92, "Video system"},
    {0x93, "Occean/Acclaim"},
    {0x95, "Varie"},
    {0x96, "Yonezawa/s’pal"},
    {0x97, "Kaneko"},
    {0x99, "Pack in soft"},
    {0xA4, "Konami (Yu-Gi-Oh!)"}
    };

std::string cart_lic_name() {
    if(context.header->new_lic_code <= 0xA4)
        return lic_codes.at(context.header->lic_code);
    return "Unknown";
}

std::string rom_type() {
    if(context.header->type <= 0x22)
        return rom_types[context.header->type];
    return "Unknown";
}


bool cart_load (std::string cart) {
    context.filename = cart;
    std::ifstream file(cart, std::ios::in | std::ios::binary | std::ios::ate);
    if(!file.is_open()) {
        std::cout << "Failed to open, please ensure the file type and directory passed is correct. Make sure you include the file extension." << std::endl;
        return false;
    }
    
    std::cout << "Opening: " << cart <<  "...\nSUCCESS" << std::endl;
    
    context.rom_size = (unsigned long)file.tellg();
    file.seekg(0, std::ios::beg);
    context.rom_data = new char[context.rom_size];
    file.read(context.rom_data, context.rom_size);
    file.close();
    context.header = (rom_header *)(context.rom_data + 0x100);

    std::cout << "Cartridge loaded" << std::endl;
    std::cout << "  Title: " << context.header->title << std::endl;
    std::cout << "  Type: "  << context.header->type << "(" << rom_type() << ")" << std::endl;
    std::cout << "  ROM Size: " << (32 << context.header->rom_size) << " KB" << std::endl;
    std::cout << "  Ram Size: "  << context.header->ram_size << std::endl;
    std::cout << "  LIC Code: "  << context.header->lic_code << "(" << cart_lic_name() << ")" << std::endl;
    std::cout << "  Rom Version: "  << context.header->version << std::endl;

    unsigned short x = 0;
    for(unsigned short i = 0x0134; i <= 0x014C; i++)
        x = x - context.rom_data[i] - 1;
    std::cout << "  Checksum: " << std::hex << x;
    if(x & 0xFF)
        std::cout << " PASSED" << std::endl;
    else
        std::cout << " FAILED" << std::endl;
    return true;
}
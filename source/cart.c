#include "cart.h"

u8* rom = NULL;
u8* sram = NULL;

u8 mbc = 0;

u32 romBank0 = 0;
u32 romBank1 = 1;
u32 ramBank0 = 0;
u32 ramBank1 = 1;

u8 readCart (u16 address) {
    return rom[address];
}
void writeCart (u16 address, u8 value) {
    // TODO: Cart logic
    (void)address;
    (void)value;
}

u8 readSRAM (u16 address) {
    // TODO: Cart logic
    (void)address;
    return 0xff;
}
void writeSRAM (u16 address, u8 value) {
    // TODO: Cart logic
    (void)address;
    (void)value;
}

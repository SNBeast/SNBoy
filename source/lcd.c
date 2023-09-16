#include "lcd.h"

u8 vram[0x2000] = {0};
u8 oam[0xa0] = {0};

union LCDC_Union lcdc_union = {.lcdc = 0x91};
union STAT_Union stat_union = {.stat = 0x81};

union Palette_Union BGP = {.palette = 0xfc};
union Palette_Union OBP1 = {0};
union Palette_Union OBP2 = {0};

u8 SCX = 0;
u8 SCY = 0;
u8 WX = 0;
u8 WY = 0;
u8 LY = 0x90;
u8 LYC = 0;

u8 readVRAM (u16 address) {
    return vram[address];
}
void writeVRAM (u16 address, u8 value) {
    vram[address] = value;
}

u8 readOAM (u16 address) {
    if (address < 0xa0) {
        return oam[address];
    }
    else {
        // wild west
        return 0;
    }
}
void writeOAM (u16 address, u8 value) {
    if (address < 0xa0) {
        oam[address] = value;
    }
}

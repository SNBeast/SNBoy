#include "motherboard.h"
#include "cart.h"
#include "cpu.h"
#include "lcd.h"

u8 wram[0x2000] = {0};
u8 hram[0x7f] = {0};

bool wramAccessible = 1;

u8 read8 (u16 address) {
    // Cart ROM
    if (address < 0x8000) {
        return readCart(address);
    }
    // VRAM
    else if (address < 0xa000) {
        return readVRAM(address - 0x8000);
    }
    // Cart RAM
    else if (address < 0xc000) {
        return readSRAM(address - 0xa000);
    }
    // WRAM
    else if (address < 0xe000) {
        if (wramAccessible) {
            return wram[address - 0xc000];
        }
        else {
            return 0xff;
        }
    }
    // Echo WRAM (Nintendo-forbidden)
    else if (address < 0xfe00) {
        if (wramAccessible) {
            return wram[address - 0xe000];
        }
        else {
            return 0xff;
        }
    }
    // OAM
    else if (address < 0xff00) {
        return readOAM(address - 0xfe00);
    }
    // device control
    else if (address < 0xff80 || address == 0xffff) {
        switch (address) {
            case 0xff00:
                // TODO: Joypad
                return 0xcf;
            case 0xff01:
                // TODO: Serial
                return 0x00;
            case 0xff02:
                // TODO: Serial
                return 0x7e;
            case 0xff04:
                // TODO: Timers
                return 0xab;
            case 0xff05:
                // TODO: Timers
                return 0x00;
            case 0xff06:
                // TODO: Timers
                return 0x00;
            case 0xff07:
                // TODO: Timers
                return 0xf8;
            case 0xff0f:
                return interrupt_flags.interruptFlags;
            case 0xff10:
                // TODO: Sound
                return 0x80;
            case 0xff11:
                // TODO: Sound
                return 0xbf;
            case 0xff12:
                // TODO: Sound
                return 0xf3;
            case 0xff13:
                // TODO: Sound
                return 0xff;
            case 0xff14:
                // TODO: Sound
                return 0xbf;
            case 0xff16:
                // TODO: Sound
                return 0x3f;
            case 0xff17:
                // TODO: Sound
                return 0x00;
            case 0xff18:
                // TODO: Sound
                return 0xff;
            case 0xff19:
                // TODO: Sound
                return 0xbf;
            case 0xff1a:
                // TODO: Sound
                return 0x7f;
            case 0xff1b:
                // TODO: Sound
                return 0xff;
            case 0xff1c:
                // TODO: Sound
                return 0x9f;
            case 0xff1d:
                // TODO: Sound
                return 0xff;
            case 0xff1e:
                // TODO: Sound
                return 0xbf;
            case 0xff20:
                // TODO: Sound
                return 0xff;
            case 0xff21:
                // TODO: Sound
                return 0x00;
            case 0xff22:
                // TODO: Sound
                return 0x00;
            case 0xff23:
                // TODO: Sound
                return 0xbf;
            case 0xff24:
                // TODO: Sound
                return 0x77;
            case 0xff25:
                // TODO: Sound
                return 0xf3;
            case 0xff26:
                // TODO: Sound
                return 0xf1;
            case 0xff40:
                return lcdc_union.lcdc;
            case 0xff41:
                return stat_union.stat;
            case 0xff42:
                return SCY;
            case 0xff43:
                return SCX;
            case 0xff44:
                return LY;
            case 0xff45:
                return LYC;
            case 0xff46:
                // TODO: OAM DMA
                return 0xff;
            case 0xff47:
                return BGP.palette;
            case 0xff48:
                return OBP1.palette;
            case 0xff49:
                return OBP2.palette;
            case 0xff4a:
                return WY;
            case 0xff4b:
                return WX;
            case 0xffff:
                return interrupt_enable.interruptFlags;
        }
        return 0xff;
    }
    // HRAM
    else {
        return hram[address - 0xff80];
    }
}

void write8 (u16 address, u8 value) {
    // Cart ROM
    if (address < 0x8000) {
        writeCart(address, value);
    }
    // VRAM
    else if (address < 0xa000) {
        writeVRAM(address - 0x8000, value);
    }
    // Cart RAM
    else if (address < 0xc000) {
        writeSRAM(address - 0xa000, value);
    }
    // WRAM
    else if (address < 0xe000) {
        if (wramAccessible) {
            wram[address - 0xc000] = value;
        }
    }
    // Echo WRAM (Nintendo-forbidden)
    else if (address < 0xfe00) {
        if (wramAccessible) {
            wram[address - 0xe000] = value;
        }
    }
    // OAM
    else if (address < 0xff00) {
        writeOAM(address - 0xfe00, value);
    }
    // device control
    else if (address < 0xff80 || address == 0xffff) {
        switch (address) {
            case 0xff00:
                // TODO: Joypad
                break;
            case 0xff01:
                // TODO: Serial
                break;
            case 0xff02:
                // TODO: Serial
                break;
            case 0xff04:
                // TODO: Timers
                break;
            case 0xff05:
                // TODO: Timers
                break;
            case 0xff06:
                // TODO: Timers
                break;
            case 0xff07:
                // TODO: Timers
                break;
            case 0xff0f:
                interrupt_flags.interruptFlags = 0xe0 | value;
                break;
            case 0xff10:
                // TODO: Sound
                break;
            case 0xff11:
                // TODO: Sound
                break;
            case 0xff12:
                // TODO: Sound
                break;
            case 0xff13:
                // TODO: Sound
                break;
            case 0xff14:
                // TODO: Sound
                break;
            case 0xff16:
                // TODO: Sound
                break;
            case 0xff17:
                // TODO: Sound
                break;
            case 0xff18:
                // TODO: Sound
                break;
            case 0xff19:
                // TODO: Sound
                break;
            case 0xff1a:
                // TODO: Sound
                break;
            case 0xff1b:
                // TODO: Sound
                break;
            case 0xff1c:
                // TODO: Sound
                break;
            case 0xff1d:
                // TODO: Sound
                break;
            case 0xff1e:
                // TODO: Sound
                break;
            case 0xff20:
                // TODO: Sound
                break;
            case 0xff21:
                // TODO: Sound
                break;
            case 0xff22:
                // TODO: Sound
                break;
            case 0xff23:
                // TODO: Sound
                break;
            case 0xff24:
                // TODO: Sound
                break;
            case 0xff25:
                // TODO: Sound
                break;
            case 0xff26:
                // TODO: Sound
                break;
            case 0xff40:
                lcdc_union.lcdc = value;
                break;
            case 0xff41:
                stat_union.stat = 0x80 | (value & 0xfc) | stat_union.mode;
                break;
            case 0xff42:
                SCY = value;
                break;
            case 0xff43:
                SCX = value;
                break;
            case 0xff44:
                // LY is read only
                break;
            case 0xff45:
                LYC = value;
                break;
            case 0xff46:
                // TODO: OAM DMA
                break;
            case 0xff47:
                BGP.palette = value;
                break;
            case 0xff48:
                OBP1.palette = value;
                break;
            case 0xff49:
                OBP2.palette = value;
                break;
            case 0xff4a:
                WY = value;
                break;
            case 0xff4b:
                WX = value;
                break;
            case 0xffff:
                interrupt_enable.interruptFlags = value;
                break;
        }
    }
    // HRAM
    else {
        hram[address - 0xff80] = value;
    }
}

u16 read16 (u16 address) {
    u16 retVal = read8(address);
    return ((u16)read8(address + 1) << 8) | retVal;
}

void write16 (u16 address, u16 value) {
    write8(address, value & 0xff);
    write8(address + 1, value >> 8);
}

void write16_reverse (u16 address, u16 value) {
    write8(address + 1, value >> 8);
    write8(address, value & 0xff);
}

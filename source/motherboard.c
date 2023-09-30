#include "motherboard.h"
#include "cart.h"
#include "cpu.h"
#include "lcd.h"
#include "serial.h"
#include "timers.h"

#include <SDL2/SDL.h>

#include <string.h>

u8 wram[0x2000] = {};
u8 hram[0x7f] = {};

u8 const* keys;
bool actionIgnored;
bool dpadIgnored;

bool wramAccessible;
bool stopMode;

void tickFromCPU (void) {
    tick_timers();
    serial_tick();
    lcd_step();
}

u8 read8 (u16 address, bool cpu) {
    // Cart ROM
    if (address < 0x8000) {
        return readCart(address);
    }
    // VRAM
    else if (address < 0xa000) {
        return readVRAM(address - 0x8000, cpu);
    }
    // Cart RAM
    else if (address < 0xc000) {
        return readSRAM(address - 0xa000);
    }
    // WRAM
    else if (address < 0xe000) {
        if (wramAccessible || !cpu) {
            return wram[address - 0xc000];
        }
        else {
            return 0xff;
        }
    }
    // Echo WRAM (Nintendo-forbidden)
    else if (address < 0xfe00) {
        if (wramAccessible || !cpu) {
            return wram[address - 0xe000];
        }
        else {
            return 0xff;
        }
    }
    // OAM
    else if (address < 0xff00) {
        return readOAM(address - 0xfe00, cpu);
    }
    // device control
    else if (address < 0xff80 || address == 0xffff) {
        switch (address) {
            case 0xff00:
                return 0xc0 | (actionIgnored << 5) | (dpadIgnored << 4)
                            | (!((!actionIgnored && keys[SDL_SCANCODE_RETURN]) || (!dpadIgnored && keys[SDL_SCANCODE_DOWN])) << 3)
                            | (!((!actionIgnored && keys[SDL_SCANCODE_TAB]) || (!dpadIgnored && keys[SDL_SCANCODE_UP])) << 2)
                            | (!((!actionIgnored && keys[SDL_SCANCODE_Z]) || (!dpadIgnored && keys[SDL_SCANCODE_LEFT])) << 1)
                            | !((!actionIgnored && keys[SDL_SCANCODE_X]) || (!dpadIgnored && keys[SDL_SCANCODE_RIGHT]));
            case 0xff01:
                return sb;
            case 0xff02:
                return sc_union.sc;
            case 0xff04:
                return DIV;
            case 0xff05:
                return TIMA;
            case 0xff06:
                return TMA;
            case 0xff07:
                return tac_union.tac;
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

void write8 (u16 address, u8 value, bool cpu) {
    if (cpu) {
        tickFromCPU();
    }
    // Cart ROM
    if (address < 0x8000) {
        writeCart(address, value);
    }
    // VRAM
    else if (address < 0xa000) {
        writeVRAM(address - 0x8000, value, cpu);
    }
    // Cart RAM
    else if (address < 0xc000) {
        writeSRAM(address - 0xa000, value);
    }
    // WRAM
    else if (address < 0xe000) {
        if (wramAccessible || !cpu) {
            wram[address - 0xc000] = value;
        }
    }
    // Echo WRAM (Nintendo-forbidden)
    else if (address < 0xfe00) {
        if (wramAccessible || !cpu) {
            wram[address - 0xe000] = value;
        }
    }
    // OAM
    else if (address < 0xff00) {
        writeOAM(address - 0xfe00, value, cpu);
    }
    // device control
    else if (address < 0xff80 || address == 0xffff) {
        switch (address) {
            case 0xff00:
                actionIgnored = value & (1 << 5);
                dpadIgnored = value & (1 << 4);
                break;
            case 0xff01:
                sb = value;
                break;
            case 0xff02:
                sc_union.sc = 0x7e | value;
                if (!sc_union.shiftClock || !sc_union.transferActive) {
                    serial_bit_counter = 0;
                }
                break;
            case 0xff04:
                DIV = 0;
                break;
            case 0xff05:
                TIMA = value;
                break;
            case 0xff06:
                TMA = value;
                break;
            case 0xff07:
                tac_union.tac = 0xf8 | value;
                tima_counter = 0;
                break;
            case 0xff0f:
                interrupt_flags.interruptFlags = value;
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
                if (value < 0xe0) {
                    for (int i = 0; i < 0xa0; i++) {
                        writeOAM(i, read8((value << 8) | i, 0), 0);
                    }
                }
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

u16 read16 (u16 address, bool cpu) {
    u16 retVal = read8(address, cpu);
    return ((u16)read8(address + 1, cpu) << 8) | retVal;
}

void write16 (u16 address, u16 value, bool cpu) {
    write8(address, value & 0xff, cpu);
    write8(address + 1, value >> 8, cpu);
}

void write16_reverse (u16 address, u16 value, bool cpu) {
    write8(address + 1, value >> 8, cpu);
    write8(address, value & 0xff, cpu);
}

#include "motherboard.h"
#include "apu.h"
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
    apu_tick();
}

u8 read8 (u16 address, bool cpu) {
    if (cpu) {
        tickFromCPU();
    }
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
                return nr10_union.nr10;
            case 0xff11:
                return nr11_union.nr11 | 0x3f;
            case 0xff12:
                return nr12_union.nr12;
            case 0xff13:
                return 0xff;
            case 0xff14:
                return nr14_union.nr14 | 0x87;
            case 0xff16:
                return nr21_union.nr11 | 0x3f;
            case 0xff17:
                return nr22_union.nr12;
            case 0xff18:
                return 0xff;
            case 0xff19:
                return nr24_union.nr14 | 0x87;
            case 0xff1a:
                return nr30_union.nr30;
            case 0xff1b:
                return 0xff;
            case 0xff1c:
                return nr32_union.nr32;
            case 0xff1d:
                return 0xff;
            case 0xff1e:
                return nr34_union.nr14 | 0x87;
            case 0xff20:
                return 0xff;
            case 0xff21:
                return nr42_union.nr12;
            case 0xff22:
                return nr43_union.nr43;
            case 0xff23:
                return nr44_union.nr44 | 0x80;
            case 0xff24:
                return nr50_union.nr50;
            case 0xff25:
                return nr51_union.nr51;
            case 0xff26:
                return nr52_union.nr52;
            case 0xff30:
            case 0xff31:
            case 0xff32:
            case 0xff33:
            case 0xff34:
            case 0xff35:
            case 0xff36:
            case 0xff37:
            case 0xff38:
            case 0xff39:
            case 0xff3a:
            case 0xff3b:
            case 0xff3c:
            case 0xff3d:
            case 0xff3e:
            case 0xff3f:
                return waveRAM[address - 0xff30];
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
                nr10_union.nr10 = value | 0x80;
                break;
            case 0xff11:
                nr11_union.nr11 = value;
                break;
            case 0xff12:
                nr12_union.nr12 = value;
                nr52_union.channelOne = nr12_union.nr12 & 0xf8;
                break;
            case 0xff13:
                nr13 = value;
                break;
            case 0xff14:
                nr14_union.nr14 = value | 0x38;
                break;
            case 0xff16:
                nr21_union.nr11 = value;
                break;
            case 0xff17:
                nr22_union.nr12 = value;
                channelTwoDAC = nr22_union.nr12 & 0xf8;
                break;
            case 0xff18:
                nr23 = value;
                break;
            case 0xff19:
                nr24_union.nr14 = value | 0x38;
                break;
            case 0xff1a:
                nr30_union.dac = value & 0x80;
                break;
            case 0xff1b:
                nr31 = value;
                break;
            case 0xff1c:
                nr32_union.nr32 = value | 0x9f;
                break;
            case 0xff1d:
                nr33 = value;
                break;
            case 0xff1e:
                nr34_union.nr14 = value | 0x38;
                break;
            case 0xff20:
                nr41_union.nr41 = value | 0xc0;
                break;
            case 0xff21:
                nr42_union.nr12 = value;
                channelFourDAC = nr42_union.nr12 & 0xf8;
                break;
            case 0xff22:
                nr43_union.nr43 = value;
                break;
            case 0xff23:
                nr44_union.nr44 = value | 0x3f;
                break;
            case 0xff24:
                nr50_union.nr50 = value;
                break;
            case 0xff25:
                nr51_union.nr51 = value;
                break;
            case 0xff26:
                nr52_union.apuEnable = value & 0x80;
                break;
            case 0xff30:
            case 0xff31:
            case 0xff32:
            case 0xff33:
            case 0xff34:
            case 0xff35:
            case 0xff36:
            case 0xff37:
            case 0xff38:
            case 0xff39:
            case 0xff3a:
            case 0xff3b:
            case 0xff3c:
            case 0xff3d:
            case 0xff3e:
            case 0xff3f:
                waveRAM[address - 0xff30] = value;
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

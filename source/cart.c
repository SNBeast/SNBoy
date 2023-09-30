#include "cart.h"

u8* rom = NULL;
u8* sram = NULL;

size_t romSize;
size_t savSize;

size_t romMagnitude;
size_t savMagnitude;

u8 mbc;
bool mbc1m;

size_t romBank0;
size_t romBank1;
size_t ramBank0;
size_t ramBank1;

bool sramEnabled;

bool mbcBool;
u8 mbcVar1;
u8 mbcVar2;

u8 latchedRTCValue[5] = {};

void mbc1ROMWrite (u16 address, u8 value) {
    if (address < 0x2000) {
        sramEnabled = (value & 0x0f) == 0x0a;
    }
    else if (address < 0x4000) {
        u8 temp = value & 0x1f;
        if (!temp) {
            temp = 1;
        }
        if (mbc1m) {
            temp &= 0x0f;
        }
        if (mbc1m) {
            mbcVar1 = (mbcVar1 & 0xf0) | temp;
        }
        else {
            mbcVar1 = (mbcVar1 & 0xe0) | temp;
        }
        romBank1 = mbcVar1;
    }
    else if (address < 0x6000) {
        mbcVar2 = value & 0x03;
        if (romSize > 0x80000) {
            if (mbc1m) {
                if (mbcBool) {
                    romBank0 = mbcVar2 << 4;
                }
                mbcVar1 = (mbcVar1 & 0xcf) | (mbcVar2 << 4);
            }
            else {
                if (mbcBool) {
                    romBank0 = mbcVar2 << 5;
                }
                mbcVar1 = (mbcVar1 & 0x9f) | (mbcVar2 << 5);
            }
            romBank1 = mbcVar1;
        }
        else {
            if (mbcBool) {
                ramBank0 = mbcVar2 << 1;
                ramBank1 = (mbcVar2 << 1) | 1;
            }
        }
    }
    else {
        mbcBool = value & 0x01;
        if (mbcBool) {
            if (romSize > 0x80000) {
                if (mbc1m) {
                    romBank0 = mbcVar2 << 4;
                }
                else {
                    romBank0 = mbcVar2 << 5;
                }
            }
            else {
                ramBank0 = mbcVar2 << 1;
                ramBank1 = (mbcVar2 << 1) | 1;
            }
        }
        else {
            romBank0 = 0;
            ramBank0 = 0;
            ramBank1 = 1;
        }
    }
}

void mbc2ROMWrite (u16 address, u8 value) {
    if (address < 0x4000) {
        if (address & 0x100) {
            romBank1 = value & 0x0f;
            if (!romBank1) {
                romBank1 = 1;
            }
        }
        else {
            sramEnabled = (value & 0x0f) == 0x0a;
        }
    }
}

void mbc3ROMWrite (u16 address, u8 value) {
    if (address < 0x2000) {
        sramEnabled = (value & 0x0f) == 0x0a;
    }
    else if (address < 0x4000) {
        romBank1 = value & 0x7f;
        if (!romBank1) {
            romBank1 = 1;
        }
    }
    else if (address < 0x6000) {
        value &= 0x0f;
        ramBank0 = value << 1;
        ramBank1 = (value << 1) | 1;
    }
    else {
        if (!mbcVar1 && value == 1) {
            mbcBool ^= 1;
            // TODO: RTC LATCH
        }
        mbcVar1 = value;
    }
}

void mbc5ROMWrite (u16 address, u8 value) {
    if (address < 0x2000) {
        sramEnabled = (value & 0x0f) == 0x0a;
    }
    else if (address < 0x3000) {
        mbcVar1 = value;
        romBank0 = (mbcBool << 8) | mbcVar1;
    }
    else if (address < 0x4000) {
        mbcBool = value & 0x01;
        romBank0 = (mbcBool << 8) | mbcVar1;
    }
    else if (address < 0x6000) {
        ramBank0 = value << 1;
        ramBank1 = (value << 1) | 1;
    }
}

u8 rtcRead (void) {
    // TODO: RTC
    return 0xff;
}

void rtcWrite (u8 value) {
    // TODO: RTC
    (void)value;
}

// Just gonna pretend for now that other MBCs and peripherals in carts don't exist.

u8 readCart (u16 address) {
    size_t actualAddress;
    if (address < 0x4000) {
        actualAddress = romBank0 * 0x4000 + address;
    }
    else {
        actualAddress = romBank1 * 0x4000 + (address - 0x4000);
    }
    actualAddress &= (1 << romMagnitude) - 1;
    if (actualAddress < romSize) {
        return rom[actualAddress];
    }
    return 0xff;
}
void writeCart (u16 address, u8 value) {
    switch (mbc) {
        case 0x01:
        case 0x02:
        case 0x03:
            mbc1ROMWrite(address, value);
            break;
        case 0x05:
        case 0x06:
            mbc2ROMWrite(address, value);
            break;
        case 0x0f:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            mbc3ROMWrite(address, value);
            break;
        case 0x19:
        case 0x1a:
        case 0x1b:
        case 0x1c:
        case 0x1d:
        case 0x1e:
            mbc5ROMWrite(address, value);
            break;
    }
}

u8 readSRAM (u16 address) {
    if (sramEnabled) {
        size_t actualAddress;
        if (address < 0x1000) {
            actualAddress = ramBank0 * 0x1000 + address;
        }
        else {
            actualAddress = ramBank1 * 0x1000 + (address - 0x1000);
        }
        actualAddress &= (1 << savMagnitude) - 1;
        if (actualAddress < savSize) {
            return sram[actualAddress];
        }
    }
    return 0xff;
}
void writeSRAM (u16 address, u8 value) {
    if (sramEnabled) {
        size_t actualAddress;
        if (address < 0x1000) {
            actualAddress = ramBank0 * 0x1000 + address;
        }
        else {
            actualAddress = ramBank1 * 0x1000 + (address - 0x1000);
        }
        actualAddress &= (1 << savMagnitude) - 1;
        if (actualAddress < savSize) {
            sram[actualAddress] = value;
        }
    }
}

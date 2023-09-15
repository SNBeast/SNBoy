#include "common.h" // shut up vscode errors
#include "memory.h"

u8 read8 (u16 address) {
    return 0;
}

void write8 (u16 address, u8 value) {

}

u16 read16 (u16 address) {
    u16 retVal = read8(address);
    return ((u16)read8(address + 1) << 8) | retVal;
}

void write16 (u16 address, u16 value) {
    write8(address, value & 0xff);
    write8(address + 1, value >> 8);
}

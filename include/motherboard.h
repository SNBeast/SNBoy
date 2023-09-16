#ifndef MOTHERBOARD_H
#define MOTHERBOARD_H

#include "common.h"

extern u8 read8(u16 address);
extern void write8 (u16 address, u8 value);
extern u16 read16 (u16 address);
extern void write16 (u16 address, u16 value);
extern void write16_reverse (u16 address, u16 value);

#endif // MOTHERBOARD_H

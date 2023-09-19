#ifndef MOTHERBOARD_H
#define MOTHERBOARD_H

#include "common.h"

extern bool stopMode;

extern u8 read8(u16 address, bool cpu);
extern void write8 (u16 address, u8 value, bool cpu);
extern u16 read16 (u16 address, bool cpu);
extern void write16 (u16 address, u16 value, bool cpu);
extern void write16_reverse (u16 address, u16 value, bool cpu);

#endif // MOTHERBOARD_H

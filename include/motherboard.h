#ifndef MOTHERBOARD_H
#define MOTHERBOARD_H

#include "common.h"

extern bool stopMode;

extern void tickFromCPU(void);

extern u8 read8(u16 address, bool cpu);
extern void write8 (u16 address, u8 value, bool cpu);
extern u16 read16 (u16 address, bool cpu);
extern void write16 (u16 address, u16 value, bool cpu);
extern void write16_reverse (u16 address, u16 value, bool cpu);

extern u8 const* keys;

extern bool actionIgnored;
extern bool dpadIgnored;

extern bool wramAccessible;
extern bool stopMode;

#endif // MOTHERBOARD_H

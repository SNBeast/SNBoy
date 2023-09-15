#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

u8 read8(u16 address);
void write8 (u16 address, u8 value);
u16 read16 (u16 address);
void write16 (u16 address, u16 value);

#endif // MEMORY_H

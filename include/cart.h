#ifndef CART_H
#define CART_H

#include "common.h"

extern u8* rom;
extern u8* sram;

extern u8 readCart(u16 address);
extern void writeCart(u16 address, u8 value);

extern u8 readSRAM(u16 address);
extern void writeSRAM(u16 address, u8 value);

#endif // CART_H

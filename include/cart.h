#ifndef CART_H
#define CART_H

#include "common.h"

extern u8* rom;
extern u8* sram;

extern size_t romSize;
extern size_t savSize;

extern size_t romMagnitude;
extern size_t savMagnitude;

extern u8 mbc;
extern bool mbc1m;

extern size_t romBank0;
extern size_t romBank1;
extern size_t ramBank0;
extern size_t ramBank1;

extern bool sramEnabled;
extern bool mbcBool;
extern u8 mbcVar1;
extern u8 mbcVar2;

extern u8 readCart(u16 address);
extern void writeCart(u16 address, u8 value);

extern u8 readSRAM(u16 address);
extern void writeSRAM(u16 address, u8 value);

#endif // CART_H

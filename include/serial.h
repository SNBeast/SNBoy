#ifndef SERIAL_H
#define SERIAL_H

#include "common.h"

union SC_Union {
    u8 sc;
    struct {
        bool shiftClock:1;
        u8 alwaysOn:6;
        bool transferActive:1;
    };
};

extern union SC_Union sc_union;
extern u8 sb;

extern u8 serial_bit_counter;
extern u8 serial_tick_counter;

extern void serial_tick(void);

static void serial_assertions (void) __attribute__ ((unused));
static void serial_assertions (void) {
    static_assert(sizeof(union SC_Union) == sizeof(u8));
}

#endif // SERIAL_H

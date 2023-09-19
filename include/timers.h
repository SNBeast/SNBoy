#ifndef TIMERS_H
#define TIMERS_H

#include "common.h"

union TAC_Union {
    u8 tac;
    struct {
        u8 input_clock:2;
        bool timer_enable:1;
        u8 alwaysOn:5;
    };
};

static void timer_assertions (void) __attribute__ ((unused));
static void timer_assertions (void) {
    static_assert(sizeof(union TAC_Union) == sizeof(u8));
}

extern union TAC_Union tac_union;

extern u8 DIV; // apparently "div" conflicts with internal compiler stuff
extern u8 TIMA;
extern u8 TMA;

extern u16 div_counter;
extern u16 tima_counter;

extern void tick_timers(void);

#endif // TIMERS_H

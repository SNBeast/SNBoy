#ifndef CPU_H
#define CPU_H

#include "common.h"

struct CPU_State {
    u16 pc;
    u16 sp;
    union {
        u16 af;
        struct {
            union {
                u8 f;
                struct {
                    u8 unused_flags:4;
                    bool carry:1;
                    bool half_carry:1;
                    bool subtract:1;
                    bool zero:1;
                };
            };
            u8 a;
        };
    };
    union {
        u16 bc;
        struct {
            u8 c;
            u8 b;
        };
    };
    union {
        u16 de;
        struct {
            u8 e;
            u8 d;
        };
    };
    union {
        u16 hl;
        struct {
            u8 l;
            u8 h;
        };
    };
    bool ime;
};

union InterruptFlags {
    u8 interruptFlags;
    struct {
        bool vblank:1;
        bool stat:1;
        bool timer:1;
        bool serial:1;
        bool joypad:1;
        u8 alwaysOn:3;
    };
};

static void cpu_assertions (void) __attribute__ ((unused));
static void cpu_assertions (void) {
    static_assert(sizeof(union InterruptFlags) == sizeof(u8));
}

extern struct CPU_State cpu;
extern union InterruptFlags interrupt_flags;
extern union InterruptFlags interrupt_enable;

extern void cpu_step(void);

#endif // CPU_H

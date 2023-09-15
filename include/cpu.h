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

extern struct CPU_State cpu;

#endif // CPU_H

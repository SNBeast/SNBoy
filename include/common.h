#ifndef COMMON_H
#define COMMON_H

// for simplicity, only allow little-endian
#ifndef _WIN32 // Windows is always little-endian and doesn't necessarily have endian.h
    #ifdef __APPLE__
        #include <machine/endian.h>
    #else
        #include <endian.h>
    #endif // __APPLE__
    #if __BYTE_ORDER != __LITTLE_ENDIAN
        #error "Target is not little-endian."
    #endif // __BYTE_ORDER != __LITTLE_ENDIAN
#endif // _WIN32

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;

// sanity checks
union Test_Alignment {
    u16 A;
    struct {
        u8 a;
        u8 b;
    };
};

union Test_Bitfields {
    u8 A;
    struct {
        u8 unused_flags:4;
        bool carry:1;
        bool half_carry:1;
        bool subtract:1;
        bool zero:1;
    };
};

static void common_assertions (void) __attribute__ ((unused));
static void common_assertions (void) {
    static_assert(sizeof(union Test_Alignment) == sizeof(u16));
    static_assert(sizeof(union Test_Bitfields) == sizeof(u8));
}

#endif // COMMON_H

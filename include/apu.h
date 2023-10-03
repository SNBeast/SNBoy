#ifndef APU_H
#define APU_H

#include "common.h"

union NR10_Union {
    u8 nr10;
    struct {
        u8 sweepSlope:3;
        bool sweepSubtract:1;
        u8 sweepPace:3;
        bool unused:1;
    };
};

union NR11_Union {
    u8 nr11;
    struct {
        u8 lengthInit:6;
        u8 dutyCycle:2;
    };
};

union NR12_Union {
    u8 nr12;
    struct {
        u8 sweepPace:3;
        bool envelopeIncrease:1;
        u8 volumeInit:4;
    };
};

union NR14_Union {
    u8 nr14;
    struct {
        u8 periodHigh:3;
        u8 unused:3;
        bool lengthEnable:1;
        bool channelRestart:1;
    };
};

union NR30_Union {
    u8 nr30;
    struct {
        u8 unused:7;
        bool dac:1;
    };
};

union NR32_Union {
    u8 nr32;
    struct {
        u8 unused:5;
        u8 outputLevel:2;
        bool unusedBit:1;
    };
};

union NR41_Union {
    u8 nr41;
    struct {
        u8 length:6;
        u8 unused:2;
    };
};

union NR43_Union {
    u8 nr43;
    struct {
        u8 divider:3;
        bool lfsrWidth:1;
        u8 shift:4;
    };
};

union NR44_Union {
    u8 nr44;
    struct {
        u8 unused:6;
        bool lengthEnable:1;
        bool channelRestart:1;
    };
};

union NR50_Union {
    u8 nr50;
    struct {
        u8 rightVolume:3;
        bool vinRight:1;
        u8 leftVolume:3;
        bool vinLeft:1;
    };
};

union NR51_Union {
    u8 nr51;
    struct {
        bool rightOne:1;
        bool rightTwo:1;
        bool rightThree:1;
        bool rightFour:1;
        bool leftOne:1;
        bool leftTwo:1;
        bool leftThree:1;
        bool leftFour:1;
    };
};

union NR52_Union {
    u8 nr52;
    struct {
        bool channelOne:1;
        bool channelTwo:1;
        bool channelThree:1;
        bool channelFour:1;
        u8 unused:3;
        bool apuEnable:1;
    };
};

extern union NR10_Union nr10_union;
extern union NR11_Union nr11_union;
extern union NR12_Union nr12_union;
extern u8 nr13;
extern union NR14_Union nr14_union;

extern union NR11_Union nr21_union;
extern union NR12_Union nr22_union;
extern u8 nr23;
extern union NR14_Union nr24_union;

extern union NR30_Union nr30_union;
extern u8 nr31;
extern union NR32_Union nr32_union;
extern u8 nr33;
extern union NR14_Union nr34_union;

extern union NR41_Union nr41_union;
extern union NR12_Union nr42_union;
extern union NR43_Union nr43_union;
extern union NR44_Union nr44_union;

extern union NR50_Union nr50_union;
extern union NR51_Union nr51_union;
extern union NR52_Union nr52_union;

extern u8 waveRAM[16];

extern s16 sample[2];
extern bool requestingSamplePlay;

extern u8 audioTickCounter;
extern u8 channelOneProgress;
extern u8 channelTwoProgress;
extern u8 channelThreeProgress;
extern u8 channelFourProgress;

extern u16 channelOnePeriodProgress;
extern u16 channelTwoPeriodProgress;
extern u16 channelThreePeriodProgress;
extern u16 channelFourPeriodProgress;

extern u8 channelOneEnvelopeProgress;
extern u8 channelTwoEnvelopeProgress;
extern u8 channelFourEnvelopeProgress;

extern u8 channelOneSweepProgress;

extern u8 prev_div;
extern u8 apu_div_tick_counter;

extern bool channelOneDAC;
extern bool channelTwoDAC;
extern bool channelFourDAC;

extern void apu_tick(void);

static void apu_assertions (void) __attribute__ ((unused));
static void apu_assertions (void) {
    static_assert(sizeof(union NR10_Union) == sizeof(u8));
    static_assert(sizeof(union NR11_Union) == sizeof(u8));
    static_assert(sizeof(union NR12_Union) == sizeof(u8));
    static_assert(sizeof(union NR14_Union) == sizeof(u8));
    static_assert(sizeof(union NR30_Union) == sizeof(u8));
    static_assert(sizeof(union NR32_Union) == sizeof(u8));
    static_assert(sizeof(union NR41_Union) == sizeof(u8));
    static_assert(sizeof(union NR43_Union) == sizeof(u8));
    static_assert(sizeof(union NR44_Union) == sizeof(u8));
    static_assert(sizeof(union NR50_Union) == sizeof(u8));
    static_assert(sizeof(union NR51_Union) == sizeof(u8));
    static_assert(sizeof(union NR52_Union) == sizeof(u8));
}

#endif // APU_H

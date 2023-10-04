#include "apu.h"
#include "timers.h"

#include <stdlib.h>

#define PULSE_MAGIC 0x20

union NR10_Union nr10_union = {};
union NR11_Union nr11_union = {};
union NR12_Union nr12_union = {};
u8 nr13;
union NR14_Union nr14_union = {};

union NR11_Union nr21_union = {};
union NR12_Union nr22_union = {};
u8 nr23;
union NR14_Union nr24_union = {};

union NR30_Union nr30_union = {};
u8 nr31;
union NR32_Union nr32_union = {};
u8 nr33;
union NR14_Union nr34_union = {};

union NR41_Union nr41_union = {};
union NR12_Union nr42_union = {};
union NR43_Union nr43_union = {};
union NR44_Union nr44_union = {};

union NR50_Union nr50_union = {};
union NR51_Union nr51_union = {};
union NR52_Union nr52_union = {};

u8 waveRAM[16] = {};

s16 sample[2] = {};
bool requestingSamplePlay = 0;

u8 audioTickCounter;
u8 channelOneProgress;
u8 channelTwoProgress;
u8 channelThreeProgress;
u8 channelFourProgress;

u16 channelOnePeriodProgress;
u16 channelTwoPeriodProgress;
u16 channelThreePeriodProgress;
u16 channelFourPeriodProgress;

u8 channelOneEnvelopeProgress;
u8 channelTwoEnvelopeProgress;
u8 channelFourEnvelopeProgress;

u8 channelOneSweepProgress;

bool channelOneDAC;
bool channelTwoDAC;
bool channelFourDAC;

u8 prev_div;
u8 apu_div_tick_counter;

void apu_tick (void) {
    audioTickCounter++;
    if (nr52_union.apuEnable) {
        if (nr14_union.channelRestart && channelOneDAC) {
            nr14_union.channelRestart = 0;
            nr52_union.channelOne = 1;
            channelOnePeriodProgress = (nr14_union.periodHigh << 8) | nr13;
        }
        if (nr24_union.channelRestart && channelTwoDAC) {
            nr24_union.channelRestart = 0;
            nr52_union.channelTwo = 1;
            channelTwoPeriodProgress = (nr24_union.periodHigh << 8) | nr23;
        }
        if (nr34_union.channelRestart && nr30_union.dac) {
            nr34_union.channelRestart = 0;
            nr52_union.channelThree = 1;
            channelThreeProgress = 0;
            channelThreePeriodProgress = (nr34_union.periodHigh << 8) | nr33;
        }
        if (nr44_union.channelRestart && channelFourDAC) {
            nr44_union.channelRestart = 0;
            nr52_union.channelFour = 1;
            channelFourProgress = 0;
            channelFourPeriodProgress = 0;
        }
        if (nr52_union.channelOne) {
            channelOnePeriodProgress++;
            if (channelOnePeriodProgress == 2048) {
                channelOnePeriodProgress = (nr14_union.periodHigh << 8) | nr13;
                channelOneProgress++;
            }
        }
        if (nr52_union.channelTwo) {
            channelTwoPeriodProgress++;
            if (channelTwoPeriodProgress == 2048) {
                channelTwoPeriodProgress = (nr24_union.periodHigh << 8) | nr23;
                channelTwoProgress++;
            }
        }
        if (nr52_union.channelThree) {
            channelThreePeriodProgress += 2;
            if (channelThreePeriodProgress >= 2048) {
                channelThreePeriodProgress -= 2048;
                channelThreePeriodProgress += (nr34_union.periodHigh << 8) | nr33;
                channelThreeProgress++;
                channelThreeProgress &= 0x1f;
            }
        }
        if (nr52_union.channelFour) {
            if (!(audioTickCounter & 3)) {
                channelFourPeriodProgress++;
                u16 progressLimit = 1 << nr43_union.shift;
                if (nr43_union.divider) {
                    progressLimit *= nr43_union.divider;
                }
                else {
                    progressLimit >>= 1;
                }
                if (channelFourPeriodProgress >= progressLimit) {
                    channelFourPeriodProgress = 0;
                    channelFourProgress = rand() & 1;
                }
            }
        }
        if ((prev_div & 0x10) && !(DIV & 0x10)) {
            apu_div_tick_counter++;
            if (!(apu_div_tick_counter & 1)) {
                if (nr14_union.lengthEnable && nr52_union.channelOne) {
                    nr11_union.lengthInit++;
                    if (!nr11_union.lengthInit) {
                        nr52_union.channelOne = 0;
                    }
                }
                if (nr24_union.lengthEnable && nr52_union.channelTwo) {
                    nr21_union.lengthInit++;
                    if (!nr21_union.lengthInit) {
                        nr52_union.channelTwo = 0;
                    }
                }
                if (nr34_union.lengthEnable && nr52_union.channelThree) {
                    nr31++;
                    if (!nr31) {
                        nr52_union.channelThree = 0;
                    }
                }
                if (nr44_union.lengthEnable && nr52_union.channelFour) {
                    nr41_union.length++;
                    if (!nr41_union.length) {
                        nr52_union.channelFour = 0;
                    }
                }
            }
            if (!(apu_div_tick_counter & 3)) {
                channelOneSweepProgress++;
                if (channelOneSweepProgress >= nr10_union.sweepPace) {
                    channelOneSweepProgress = 0;
                    u16 newSweep = (nr14_union.periodHigh << 8) | nr13;
                    newSweep += nr10_union.sweepSubtract ? -(newSweep >> nr10_union.sweepSlope) : (newSweep >> nr10_union.sweepSlope);
                    if (newSweep > 0x7ff) {
                        nr52_union.channelOne = 0;
                    }
                    else if (nr10_union.sweepPace) {
                        nr13 = newSweep & 0xff;
                        nr14_union.periodHigh = newSweep >> 8;
                    }
                }
            }
            if (!(apu_div_tick_counter & 7)) {
                if (nr12_union.sweepPace) {
                    channelOneEnvelopeProgress++;
                    if (channelOneEnvelopeProgress >= nr12_union.sweepPace) {
                        channelOneEnvelopeProgress = 0;
                        if (nr12_union.envelopeIncrease) {
                            if (nr12_union.volumeInit < 0xf) {
                                nr12_union.volumeInit++;
                            }
                        }
                        else {
                            if (nr12_union.volumeInit) {
                                nr12_union.volumeInit--;
                            }
                        }
                    }
                }
                if (nr22_union.sweepPace) {
                    channelTwoEnvelopeProgress++;
                    if (channelTwoEnvelopeProgress >= nr22_union.sweepPace) {
                        channelTwoEnvelopeProgress = 0;
                        if (nr22_union.envelopeIncrease) {
                            if (nr22_union.volumeInit < 0xf) {
                                nr22_union.volumeInit++;
                            }
                        }
                        else {
                            if (nr22_union.volumeInit) {
                                nr22_union.volumeInit--;
                            }
                        }
                    }
                }
                if (nr42_union.sweepPace) {
                    channelFourEnvelopeProgress++;
                    if (channelFourEnvelopeProgress >= nr42_union.sweepPace) {
                        channelFourEnvelopeProgress = 0;
                        if (nr42_union.envelopeIncrease) {
                            if (nr42_union.volumeInit < 0xf) {
                                nr42_union.volumeInit++;
                            }
                        }
                        else {
                            if (nr42_union.volumeInit) {
                                nr42_union.volumeInit--;
                            }
                        }
                    }
                }
            }
        }
        prev_div = DIV;
    }
    if (!(audioTickCounter & 0xf)) {
        s16 channelOneOut = 0;
        s16 channelTwoOut = 0;
        s16 channelThreeOut = 0;
        s16 channelFourOut = 0;

        if (nr52_union.apuEnable) {
            if (nr52_union.channelOne) {
                switch (nr11_union.dutyCycle) {
                    case 0:
                        channelOneOut = (channelOneProgress & 7) ? PULSE_MAGIC : -PULSE_MAGIC;
                        break;
                    case 1:
                    case 3:
                        channelOneOut = ((channelOneProgress & 7) < 2) ? PULSE_MAGIC : -PULSE_MAGIC;
                        break;
                    case 2:
                        channelOneOut = ((channelOneProgress & 7) < 4) ? PULSE_MAGIC : -PULSE_MAGIC;
                        break;
                }
                channelOneOut *= nr12_union.volumeInit;
            }
            if (nr52_union.channelTwo) {
                switch (nr21_union.dutyCycle) {
                    case 0:
                        channelTwoOut = (channelTwoProgress & 7) ? PULSE_MAGIC : -PULSE_MAGIC;
                        break;
                    case 1:
                    case 3:
                        channelTwoOut = ((channelTwoProgress & 7) < 2) ? PULSE_MAGIC : -PULSE_MAGIC;
                        break;
                    case 2:
                        channelTwoOut = ((channelTwoProgress & 7) < 4) ? PULSE_MAGIC : -PULSE_MAGIC;
                        break;
                }
                channelTwoOut *= nr22_union.volumeInit;
            }
            if (nr52_union.channelThree) {
                if (channelThreeProgress & 1) {
                    channelThreeOut = waveRAM[channelThreeProgress >> 1] & 0x0f;
                }
                else {
                    channelThreeOut = waveRAM[channelThreeProgress >> 1] >> 4;
                }
                channelThreeOut -= 8;
                switch (nr32_union.outputLevel) {
                    case 0:
                        channelThreeOut *= 0;
                        break;
                    case 2:
                        channelThreeOut /= 2;
                        break;
                    case 3:
                        channelThreeOut /= 4;
                        break;
                }
                channelThreeOut *= PULSE_MAGIC;
            }
            if (nr52_union.channelFour) {
                channelFourOut = channelFourProgress ? PULSE_MAGIC : -PULSE_MAGIC;
                channelFourOut *= nr42_union.volumeInit;
            }
        }

        sample[0] = 0;
        sample[1] = 0;

        if (nr51_union.leftOne) {
            sample[0] += channelOneOut;
        }
        if (nr51_union.leftTwo) {
            sample[0] += channelTwoOut;
        }
        if (nr51_union.leftThree) {
            sample[0] += channelThreeOut;
        }
        if (nr51_union.leftFour) {
            sample[0] += channelFourOut;
        }
        if (nr51_union.rightOne) {
            sample[1] += channelOneOut;
        }
        if (nr51_union.rightTwo) {
            sample[1] += channelTwoOut;
        }
        if (nr51_union.rightThree) {
            sample[1] += channelThreeOut;
        }
        if (nr51_union.rightFour) {
            sample[1] += channelFourOut;
        }

        sample[0] *= nr50_union.leftVolume + 1;
        sample[1] *= nr50_union.rightVolume + 1;

        requestingSamplePlay = 1;
    }
}

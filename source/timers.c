#include "timers.h"
#include "motherboard.h"
#include "cpu.h"

u8 DIV;
u8 TIMA;
u8 TMA;
union TAC_Union tac_union = {};

u16 div_counter;
u16 tima_counter;

void tick_timers (void) {
    if (!stopMode) {
        div_counter++;
        if (div_counter == 64) {
            DIV++;
            div_counter = 0;
        }

        if (tac_union.timer_enable) {
            tima_counter++;
            switch (tac_union.input_clock) {
                case 0:
                    if (tima_counter == 256) {
                        TIMA++;
                        tima_counter = 0;
                    }
                    break;
                case 1:
                    if (tima_counter == 4) {
                        TIMA++;
                        tima_counter = 0;
                    }
                    break;
                case 2:
                    if (tima_counter == 16) {
                        TIMA++;
                        tima_counter = 0;
                    }
                    break;
                case 3:
                    if (tima_counter == 64) {
                        TIMA++;
                        tima_counter = 0;
                    }
                    break;
            }
            if (!TIMA) {
                TIMA = TMA;
                interrupt_flags.timer = 1;
            }
        }
    }
}

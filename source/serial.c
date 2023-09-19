#include "cpu.h"
#include "serial.h"

union SC_Union sc_union = {.sc = 0x7e};
u8 sb = 0;

u8 serial_tick_counter = 0;
u8 serial_bit_counter = 0;

// actual networking will not be implemented at this time, but we need to pretend to handle internally clocked transfers so the interrupt fires
void serial_tick (void) {
    if (sc_union.shiftClock && sc_union.transferActive) {
        serial_tick_counter++;
        if (serial_tick_counter == 128) {
            sb = (sb << 1) | 1;
            serial_tick_counter = 0;
            serial_bit_counter++;
            if (serial_bit_counter == 8) {
                interrupt_flags.serial = 1;
                sc_union.transferActive = 0;
                serial_bit_counter = 0;
            }
        }
    }
}

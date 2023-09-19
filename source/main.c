#include "cart.h"
#include "cpu.h"
#include "lcd.h"
#include "motherboard.h"
#include "serial.h"
#include "timers.h"

#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv) {
    if (argc == 1) {
        puts("need path to rom");
        return 0;
    }
    FILE* romFile = fopen(argv[1], "r");

    if (!romFile) {
        puts("rom file cannot be opened");
        return 0;
    }
    if (fseek(romFile, 0, SEEK_END)) {
        puts("rom filesize cannot be determined");
        return 0;
    }
    long romSize = ftell(romFile);
    rom = malloc(romSize);
    if (!rom) {
        printf("%ld bytes could not be allocated for the rom\n", romSize);
        return 0;
    }
    if (fseek(romFile, 0, SEEK_SET)) {
        puts("rom file stream could not be reset");
        return 0;
    }
    fread(rom, 1, romSize, romFile);
    fclose(romFile);

    for (int i = 0; i < 0x00'10'00'00; i++) {
        tick_timers();
        serial_tick();
        lcd_step();
        cpu_step();
    }

    for (int i = 0; i < 144; i++) {
        for (int j = 0; j < 160; j++) {
            printf("%d", frame[i][j]);
        }
        puts("");
    }

    return 0;
}

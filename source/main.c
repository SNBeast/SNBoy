#include "cart.h"
#include "cpu.h"
#include "motherboard.h"

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

    for (int i = 0; i < 0x00'80'00'00; i++) {
        printf(
            "A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n",
            cpu.a, cpu.f, cpu.b, cpu.c, cpu.d, cpu.e, cpu.h, cpu.l, cpu.sp, cpu.pc, read8(cpu.pc), read8(cpu.pc + 1), read8(cpu.pc + 2), read8(cpu.pc + 3)
        );
        cpu_step();
    }

    return 0;
}

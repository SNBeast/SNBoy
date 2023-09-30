#include "cart.h"
#include "cpu.h"
#include "lcd.h"
#include "motherboard.h"
#include "serial.h"
#include "timers.h"

#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>

bool romLoaded = 0;
u8 previousButtons = 0xf;

void reset (void) {
    cpu.ime = 0;
    cpu.af = 0x01b0;
    cpu.bc = 0x0013;
    cpu.de = 0x00d8;
    cpu.hl = 0x014d;
    cpu.sp = 0xfffe;
    cpu.pc = 0x0100;

    interrupt_flags.interruptFlags = 0xe1;
    interrupt_enable.interruptFlags = 0;
    haltMode = 0;
    eiJustHappened = 0;
    toggleIME = 0;

    memset(vram, 0, 0x2000 * sizeof(u8));
    memset(oam, 0, 0xa0 * sizeof(u8));
    memset(objectTileColors, 4, 8 * sizeof(u8));
    memset(objectTilePriorities, 0, 8 * sizeof(bool));
    objectTileColorsPointer = 0;
    remainingSpritePixels = 0;
    SCX = 0;
    SCY = 0;
    WX = 0;
    WY = 0;
    LY = 0;
    LYC = 0;
    BGP.palette = 0xfc;
    OBP1.palette = 0;
    OBP2.palette = 0;
    lcdc_union.lcdc = 0x91;
    stat_union.stat = 0x85;
    
    actionIgnored = 0;
    dpadIgnored = 0;
    wramAccessible = 1;
    stopMode = 0;

    sc_union.sc = 0x7e;
    sb = 0;
    serial_tick_counter = 0;
    serial_bit_counter = 0;

    DIV = 0xab;
    TIMA = 0x00;
    TMA = 0x00;
    tac_union.tac = 0xf8;

    div_counter = 0;
    tima_counter = 0;

    requestFrameDraw = 0;
}

// return value: true if success
bool loadROM (const char* file) {
    FILE* romFile = fopen(file, "r");

    if (!romFile) {
        puts("ROM file cannot be opened.");
        return 0;
    }
    if (fseek(romFile, 0, SEEK_END)) {
        puts("ROM filesize cannot be determined.");
        return 0;
    }
    long romSize = ftell(romFile);
    rom = malloc(romSize);
    if (!rom) {
        printf("%ld bytes could not be allocated for the rom.\n", romSize);
        return 0;
    }
    if (fseek(romFile, 0, SEEK_SET)) {
        puts("ROM file stream could not be reset.");
        return 0;
    }
    fread(rom, 1, romSize, romFile);
    fclose(romFile);

    reset();
    romLoaded = 1;
    return 1;
}

// return value: true if exit
bool eventHandle (void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return 1;
            case SDL_DROPFILE:
                if (!loadROM(event.drop.file)) {
                    return 1;
                }
                break;
        }
    }
    return 0;
}

int main (int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("SNBoy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 432, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 160, 144);
    keys = SDL_GetKeyboardState(NULL);

    if (argc == 2) {
        loadROM(argv[1]);
    }
    else {
        while (!romLoaded) {
            SDL_Delay(100);
            if (eventHandle()) {
                return 1;
            }
        }
    }

    while (1) {
        while (!requestFrameDraw) {
            u8 currentButtons = read8(0xff00, 0) & 0x0f;
            if (currentButtons ^ previousButtons) {
                bool joypadInterruptTriggered = 0;
                for (int i = 0; i < 4; i++) {
                    if (previousButtons & (1 << i)) {
                        joypadInterruptTriggered = 1;
                    }
                }
                
                if (joypadInterruptTriggered) {
                    interrupt_flags.joypad = 1;
                    stopMode = 0;
                }

                previousButtons = currentButtons;
            }

            cpu_step();
        }
        requestFrameDraw = 0;
        void* pixels;
        int pitch;
        SDL_LockTexture(texture, NULL, &pixels, &pitch);
        for (int i = 0; i < 144; i++) {
            for (int j = 0; j < 160; j++) {
                *(u32*)((u8*)pixels + i * pitch + j * 4) = 0xff000000 | ((3 - frame[i][j]) * 0x555555);
            }
        }
        SDL_UnlockTexture(texture);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        if (eventHandle()) {
            return 0;
        }
    }

    return 0;
}

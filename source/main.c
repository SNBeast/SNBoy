#include "apu.h"
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
bool firstFrame = 0;

FILE* savFile = NULL;

// something something Sega v. Accolade, I am not a lawyer and this is not legal advice
const u8 NintendoLogo[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

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

    romBank0 = 0;
    romBank1 = 1;
    ramBank0 = 0;
    ramBank1 = 1;

    sramEnabled = 0;
    mbcBool = 0;
    mbcVar1 = 0;
    mbcVar2 = 0;

    nr10_union.nr10 = 0x80;
    nr11_union.nr11 = 0xbf;
    nr12_union.nr12 = 0xf3;
    nr13 = 0xff;
    nr14_union.nr14 = 0xbf;
    nr21_union.nr11 = 0x3f;
    nr22_union.nr12 = 0x00;
    nr23 = 0xff;
    nr24_union.nr14 = 0xbf;
    nr30_union.nr30 = 0x7f;
    nr31 = 0xff;
    nr32_union.nr32 = 0x9f;
    nr33 = 0xff;
    nr34_union.nr14 = 0xbf;
    nr41_union.nr41 = 0xff;
    nr42_union.nr12 = 0x00;
    nr43_union.nr43 = 0x00;
    nr44_union.nr44 = 0xbf;
    nr50_union.nr50 = 0x77;
    nr51_union.nr51 = 0xf3;
    nr52_union.nr52 = 0xf1;

    audioTickCounter = 0;
    prev_div = 0xab;
    channelOneProgress = 0;
    channelTwoProgress = 0;
    channelThreeProgress = 0;
    channelFourProgress = 0;
    channelOnePeriodProgress = ((nr14_union.periodHigh) << 8) | nr13;
    channelTwoPeriodProgress = ((nr24_union.periodHigh) << 8) | nr23;
    channelThreePeriodProgress = ((nr34_union.periodHigh) << 8) | nr33;
    channelFourPeriodProgress = 0;
    channelOneEnvelopeProgress = 0;
    channelTwoEnvelopeProgress = 0;
    channelFourEnvelopeProgress = 0;
    channelOneSweepProgress = 0;

    apu_div_tick_counter = 0;
}

bool closeSave (void) {
    if (fseek(savFile, 0, SEEK_SET)) {
        perror("Save file stream could not be reset.");
        return 0;
    }
    if (fwrite(sram, 1, savSize, savFile) != savSize) {
        perror("Could not write all of the save file's bytes.");
        return 0;
    }
    fclose(savFile);
    free(sram);
    return 1;
}

// return value: true if success
bool loadROM (const char* file) {
    if (sram) {
        if (!closeSave()) {
            return 0;
        }
    }
    if (rom) {
        free(rom);
    }

    FILE* romFile = fopen(file, "rb");
    if (!romFile) {
        perror("ROM file cannot be opened.");
        return 0;
    }
    if (fseek(romFile, 0, SEEK_END)) {
        perror("ROM filesize cannot be determined.");
        return 0;
    }
    romSize = ftell(romFile);
    rom = malloc(romSize);
    if (!rom) {
        fprintf(stderr, "%zu bytes could not be allocated for the ROM.\n", romSize);
        return 0;
    }
    if (fseek(romFile, 0, SEEK_SET)) {
        perror("ROM file stream could not be reset.");
        return 0;
    }
    if (fread(rom, 1, romSize, romFile) != romSize) {
        perror("Could not read all of the ROM file's bytes");
        return 0;
    }
    fclose(romFile);
    mbc = rom[0x147];
    if ((romSize > 0x40000) && (mbc == 1 || mbc == 2 || mbc == 3)) {
        mbc1m = !memcmp(rom + 0x40104, NintendoLogo, sizeof(NintendoLogo));
    }

    size_t sizeCopy = romSize;
    for (romMagnitude = 0; sizeCopy != 0; romMagnitude++) {
        sizeCopy >>= 1;
    }
    if (romSize & (romSize - 1)) {
        romMagnitude++;
    }

    char* savPath;
    if (!memcmp(file + strlen(file) - 3, ".gb", 3)) {
        savPath = malloc(strlen(file) + 2);
        memcpy(savPath, file, strlen(file) - 3);
        memcpy(savPath + strlen(file) - 3, ".sav", 5);
    }
    else if (!memcmp(file + strlen(file) - 4, ".gbc", 4)) {
        savPath = malloc(strlen(file) + 1);
        memcpy(savPath, file, strlen(file) - 4);
        memcpy(savPath + strlen(file) - 4, ".sav", 5);
    }
    else {
        savPath = malloc(strlen(file) + 5);
        memcpy(savPath, file, strlen(file));
        memcpy(savPath + strlen(file), ".sav", 5);
    }

    savFile = fopen(savPath, "rb+");
    if (!savFile) {
        switch (rom[0x149]) {
            case 0:
                savSize = 0;
                break;
            case 2:
                savSize = 0x2000;
                break;
            case 3:
                savSize = 0x8000;
                break;
            case 4:
                savSize = 0x20000;
                break;
            case 5:
                savSize = 0x10000;
                break;
        }
        if (savSize) {
            savFile = fopen(savPath, "wb+");
            if (!savFile) {
                perror("Save file cannot be opened");
                return 0;
            }
            sram = malloc(savSize);
        }
    }
    else {
        if (fseek(savFile, 0, SEEK_END)) {
            perror("Save filesize cannot be determined.");
            return 0;
        }
        savSize = ftell(savFile);
        sram = malloc(savSize);
        if (!sram) {
            fprintf(stderr, "%zu bytes could not be allocated for the save.\n", savSize);
            return 0;
        }
        if (fseek(savFile, 0, SEEK_SET)) {
            perror("ROM file stream could not be reset.");
            return 0;
        }
        if (fread(sram, 1, savSize, savFile) != savSize) {
            perror("Could not read all of the save file's bytes");
            return 0;
        }
    }

    sizeCopy = savSize;
    for (savMagnitude = 0; sizeCopy != 0; savMagnitude++) {
        sizeCopy >>= 1;
    }
    if (savSize & (savSize - 1)) {
        savMagnitude++;
    }

    free(savPath);

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
                if (sram) {
                    closeSave();
                }
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
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    SDL_AudioSpec requestedAudioSpec, realAudioSpec;
    memset(&requestedAudioSpec, 0, sizeof(SDL_AudioSpec));
    requestedAudioSpec.freq = 0x10000;
    requestedAudioSpec.format = AUDIO_S16;
    requestedAudioSpec.channels = 2;
    requestedAudioSpec.samples = 4096;
    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(NULL, 0, &requestedAudioSpec, &realAudioSpec, SDL_AUDIO_ALLOW_SAMPLES_CHANGE);

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

            if (requestingSamplePlay) {
                if (lcdc_union.lcdEnable) {
                    SDL_QueueAudio(audioDevice, sample, sizeof(sample));
                }
                requestingSamplePlay = 0;
            }
        }
        requestFrameDraw = 0;
        if (!firstFrame) {
            firstFrame = 1;
            SDL_PauseAudioDevice(audioDevice, 0);
        }
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

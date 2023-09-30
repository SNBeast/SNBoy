#include "lcd.h"
#include "cpu.h"

#include <string.h>

u8 vram[0x2000] = {};
u8 oam[0xa0] = {};

union LCDC_Union lcdc_union = {};
union STAT_Union stat_union = {};

union Palette_Union BGP = {};
union Palette_Union OBP1 = {};
union Palette_Union OBP2 = {};

u8 SCX;
u8 SCY;
u8 WX;
u8 WY;
u8 LY;
u8 LYC;

u8 objectTileColors[8] = {};
bool objectTilePriorities[8] = {};
u8 objectTileColorsPointer;
u8 remainingSpritePixels;

u8 bgTileColors[160] = {};
u8 windowTileColors[168] = {};
u8 frame[144][160] = {};

u8 line_objects[10] = {};
u8 line_object_counter = 0;
u8 line_timer = 0;

bool requestFrameDraw;

u8 readVRAM (u16 address, bool cpu) {
    if (!cpu || stat_union.stat != 3) {
        return vram[address];
    }
    return 0xff;
}
void writeVRAM (u16 address, u8 value, bool cpu) {
    if (!cpu || stat_union.stat != 3) {
        vram[address] = value;
    }
}

u8 readOAM (u16 address, bool cpu) {
    if (address < 0xa0) {
        if (!cpu || stat_union.stat < 2) {
            return oam[address];
        }
        return 0xff;
    }
    else {
        // wild west
        return 0;
    }
}
void writeOAM (u16 address, u8 value, bool cpu) {
    if (address < 0xa0) {
        if (!cpu || stat_union.stat < 2) {
            oam[address] = value;
        }
    }
}

void fetch_pixel_data (u8* pixel_buffer, bool higher_area, u8 index, u8 y) {
    u8 pixel_lsbs;
    u8 pixel_msbs;
    if (higher_area) {
        pixel_lsbs = vram[(s16)((s8)index) * 16 + 0x1000 + y * 2];
        pixel_msbs = vram[(s16)((s8)index) * 16 + 0x1000 + y * 2 + 1];
    }
    else {
        pixel_lsbs = vram[(u16)index * 16 + y * 2];
        pixel_msbs = vram[(u16)index * 16 + y * 2 + 1];
    }
    for (int i = 0; i < 8; i++) {
        pixel_buffer[7 - i] = (((pixel_msbs >> i) & 1) << 1) | ((pixel_lsbs >> i) & 1);
    }
}

void fetch_sprite_pixels (struct OAM_Entry sprite, u8 discard) {
    u8 yLine = LY + 16 - sprite.yPosition;
    if (sprite.yFlip) {
        yLine = (lcdc_union.objSize ? 15 : 7) - yLine;
    }

    u8 pixel_data[8] = {0};
    if (lcdc_union.objSize) {
        if (yLine < 8) {
            fetch_pixel_data(pixel_data, 0, sprite.tileIndex & 0xfe, yLine);
        }
        else {
            fetch_pixel_data(pixel_data, 0, sprite.tileIndex | 0x01, yLine - 8);
        }
    }
    else {
        fetch_pixel_data(pixel_data, 0, sprite.tileIndex, yLine);
    }

    u8 index = objectTileColorsPointer;
    for (int i = discard; i < 8; i++) {
        int modifiedI;
        if (sprite.xFlip) {
            modifiedI = 7 - i;
        }
        else {
            modifiedI = i;
        }
        if (objectTileColors[index] == 4 && pixel_data[modifiedI] != 4) {
            if (sprite.palette) {
                switch (pixel_data[modifiedI]) {
                    case 1:
                        objectTileColors[index] = OBP2.color1;
                        break;
                    case 2:
                        objectTileColors[index] = OBP2.color2;
                        break;
                    case 3:
                        objectTileColors[index] = OBP2.color3;
                        break;
                }
            }
            else {
                switch (pixel_data[modifiedI]) {
                    case 1:
                        objectTileColors[index] = OBP1.color1;
                        break;
                    case 2:
                        objectTileColors[index] = OBP1.color2;
                        break;
                    case 3:
                        objectTileColors[index] = OBP1.color3;
                        break;
                }
            }
        }
        objectTilePriorities[index] = sprite.bgWindowPriority;
        
        index = (index + 1) & 7;
    }

    if ((8 - discard) > remainingSpritePixels) {
        remainingSpritePixels = 8 - discard;
    }
}

void lcd_step (void) {
    if (lcdc_union.lcdEnable) {
        if (stat_union.mode != 1) {
            if (line_timer < 20) {
                if (!line_timer) {
                    for (int i = 0; i < remainingSpritePixels; i++) {
                        objectTileColors[(objectTileColorsPointer + i) & 0x7] = 4;
                    }
                    remainingSpritePixels = 0;
                    line_object_counter = 0;
                }
                if (line_object_counter < 10) {
                    u8 objectHeight = lcdc_union.objSize ? 16 : 8;
                    for (int i = 0; i < 2; i++) {
                        u8 spriteYPosition = (((struct OAM_Entry*)oam)[line_timer * 2 + i]).yPosition;
                        if (spriteYPosition <= LY + 16 && spriteYPosition > LY + (16 - objectHeight)) {
                            line_objects[line_object_counter] = line_timer * 2 + i;
                            line_object_counter++;
                            if (line_object_counter == 10) {
                                break;
                            }
                        }
                    }
                }
            }
            else {
                if (line_timer == 20) {
                    stat_union.mode = 3;

                    u8 pixel_data[256] = {0};
                    u16 background_base = lcdc_union.bgTileMapArea ? 0x1c00 : 0x1800;
                    u16 window_base = lcdc_union.windowTileMapArea ? 0x1c00 : 0x1800;

                    for (int i = 0; i < 32; i++) {
                        fetch_pixel_data(&pixel_data[i * 8], !lcdc_union.bgWindowTileDataArea, vram[(u16)(((LY + SCY) & 0xff) / 8) * 0x20 + i + background_base], (LY + SCY) & 7);
                    }
                    if (SCX < 96) {
                        memcpy(bgTileColors, pixel_data + SCX, 160);
                    }
                    else {
                        memcpy(bgTileColors, pixel_data + SCX, 160 - (SCX - 96));
                        memcpy(bgTileColors + (160 - (SCX - 96)), pixel_data, SCX - 96);
                    }
                    if (LY >= WY) {
                        for (int i = 0; i < 21; i++) {
                            fetch_pixel_data(&windowTileColors[i * 8], !lcdc_union.bgWindowTileDataArea, vram[(u16)((LY - WY) / 8) * 0x20 + i + window_base], (LY - WY) & 7);
                        }
                    }
                    for (int i = 1; i < 8; i++) {
                        for (int j = 0; j < line_object_counter; j++) {
                            struct OAM_Entry sprite = ((struct OAM_Entry*)oam)[line_objects[j]];
                            if (sprite.xPosition == i) {
                                fetch_sprite_pixels(sprite, 8 - i);
                            }
                        }
                    }
                }
                else if (line_timer == 63) {
                    stat_union.mode = 0;
                    if (stat_union.hBlankStatInterruptSource) {
                        interrupt_flags.stat = 1;
                    }
                }

                if (line_timer < 60) {
                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < line_object_counter; j++) {
                            struct OAM_Entry sprite = ((struct OAM_Entry*)oam)[line_objects[j]];
                            if (sprite.xPosition == (line_timer - 20) * 4 + i + 8) {
                                fetch_sprite_pixels(sprite, 0);
                            }
                        }
                        u8 tileColorUsed;
                        if (lcdc_union.windowEnable && LY >= WY && ((line_timer - 20) * 4 + i) + 7 >= WX) {
                            tileColorUsed = windowTileColors[((line_timer - 20) * 4 + i) + 7 - WX];
                        }
                        else {
                            tileColorUsed = bgTileColors[(line_timer - 20) * 4 + i];
                        }

                        if (remainingSpritePixels) {
                            u8 objectColor = objectTileColors[objectTileColorsPointer];
                            u8 objectPriority = objectTilePriorities[objectTileColorsPointer];
                            objectTileColors[objectTileColorsPointer] = 4;
                            objectTileColorsPointer++;
                            objectTileColorsPointer &= 7;
                            remainingSpritePixels--;
                            if (lcdc_union.objEnable && objectColor != 4 && (!objectPriority || !tileColorUsed || !lcdc_union.bgWindowEnable)) {
                                frame[LY][(line_timer - 20) * 4 + i] = objectColor;
                            }
                            else {
                                switch (tileColorUsed) {
                                    case 0:
                                        frame[LY][(line_timer - 20) * 4 + i] = BGP.color0;
                                        break;
                                    case 1:
                                        frame[LY][(line_timer - 20) * 4 + i] = BGP.color1;
                                        break;
                                    case 2:
                                        frame[LY][(line_timer - 20) * 4 + i] = BGP.color2;
                                        break;
                                    case 3:
                                        frame[LY][(line_timer - 20) * 4 + i] = BGP.color3;
                                        break;
                                }
                            }
                        }
                        else {
                            switch (tileColorUsed) {
                                case 0:
                                    frame[LY][(line_timer - 20) * 4 + i] = BGP.color0;
                                    break;
                                case 1:
                                    frame[LY][(line_timer - 20) * 4 + i] = BGP.color1;
                                    break;
                                case 2:
                                    frame[LY][(line_timer - 20) * 4 + i] = BGP.color2;
                                    break;
                                case 3:
                                    frame[LY][(line_timer - 20) * 4 + i] = BGP.color3;
                                    break;
                            }
                        }
                    }
                }
            }
        }

        line_timer++;
        if (line_timer == 114) {
            line_timer = 0;

            LY++;
            if (LY == 153) {
                LY = 0;
            }

            if (LY == LYC) {
                stat_union.lycEqLy = 1;
                if (stat_union.lycEqLyInterruptSource) {
                    interrupt_flags.stat = 1;
                }
            }
            else {
                stat_union.lycEqLy = 0;
            }

            if (LY < 144) {
                stat_union.mode = 2;
                if (stat_union.oamStatInterruptSource) {
                    interrupt_flags.stat = 1;
                }
            }
            else if (LY == 144) {
                stat_union.mode = 1;
                if (stat_union.vBlankStatInterruptSource) {
                    interrupt_flags.stat = 1;
                }
                interrupt_flags.vblank = 1;

                requestFrameDraw = 1;
            }
        }
    }
}

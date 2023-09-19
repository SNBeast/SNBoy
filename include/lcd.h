#ifndef LCD_H
#define LCD_H

#include "common.h"

union LCDC_Union {
    u8 lcdc;
    struct {
        bool bgWindowEnable:1;
        bool objEnable:1;
        bool objSize:1;
        bool bgTileMapArea:1;
        bool bgWindowTileDataArea:1;
        bool windowEnable:1;
        bool windowTileMapArea:1;
        bool lcdEnable:1;
    };
};

union STAT_Union {
    u8 stat;
    struct {
        u8 mode:2;
        bool lycEqLy:1;
        bool hBlankStatInterruptSource:1;
        bool vBlankStatInterruptSource:1;
        bool oamStatInterruptSource:1;
        bool lycEqLyInterruptSource:1;
        bool alwaysOn:1;
    };
};

union Palette_Union {
    u8 palette;
    struct {
        u8 color0:2;
        u8 color1:2;
        u8 color2:2;
        u8 color3:2;
    };
};

struct OAM_Entry {
    u8 yPosition;
    u8 xPosition;
    u8 tileIndex;
    union {
        u8 attributes;
        struct {
            u8 unused:4;
            bool palette:1;
            bool xFlip:1;
            bool yFlip:1;
            bool bgWindowPriority:1;
        };
    };
};

extern union LCDC_Union lcdc_union;
extern union STAT_Union stat_union;

extern union Palette_Union BGP;
extern union Palette_Union OBP1;
extern union Palette_Union OBP2;

extern u8 SCX;
extern u8 SCY;
extern u8 WX;
extern u8 WY;
extern u8 LY;
extern u8 LYC;

extern u8 frame[144][160];

extern u8 readVRAM(u16 address, bool cpu);
extern void writeVRAM(u16 address, u8 value, bool cpu);

extern u8 readOAM(u16 address, bool cpu);
extern void writeOAM(u16 address, u8 value, bool cpu);

extern void lcd_step(void);

extern void fetch_pixel_data(u8* pixel_buffer, bool higher_area, u8 index, u8 y);
extern u8 bgTileColors[160];

static void lcd_assertions (void) __attribute__ ((unused));
static void lcd_assertions (void) {
    static_assert(sizeof(union LCDC_Union) == sizeof(u8));
    static_assert(sizeof(union STAT_Union) == sizeof(u8));
    static_assert(sizeof(union Palette_Union) == sizeof(u8));
    static_assert(sizeof(struct OAM_Entry) == sizeof(u32));
}

#endif // LCD_H

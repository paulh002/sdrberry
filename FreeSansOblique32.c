/*******************************************************************************
 * Size: 32 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef FREESANSOBLIQUE32
#define FREESANSOBLIQUE32 1
#endif

#if FREESANSOBLIQUE32

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+002B "+" */
    0x0, 0xe0, 0x0, 0xc0, 0x0, 0xc0, 0x0, 0xc0,
    0x0, 0xc0, 0x1, 0xc0, 0x1, 0x80, 0xff, 0xff,
    0xff, 0xff, 0x3, 0x80, 0x3, 0x80, 0x3, 0x0,
    0x3, 0x0, 0x3, 0x0, 0x7, 0x0, 0x7, 0x0,

    /* U+002C "," */
    0x7b, 0xdc, 0x23, 0x11, 0x98,

    /* U+002D "-" */
    0xff, 0xff,

    /* U+002E "." */
    0xfe, 0xe0,

    /* U+0030 "0" */
    0x0, 0xf8, 0x3, 0xfe, 0x7, 0xe, 0xe, 0x7,
    0x1c, 0x7, 0x18, 0x7, 0x38, 0x7, 0x38, 0x7,
    0x70, 0x7, 0x70, 0x7, 0x70, 0x7, 0x70, 0x6,
    0x60, 0xe, 0xe0, 0xe, 0xe0, 0xe, 0xe0, 0x1c,
    0xe0, 0x1c, 0xe0, 0x18, 0xe0, 0x38, 0x70, 0x70,
    0x7f, 0xe0, 0x3f, 0xc0, 0x4, 0x0,

    /* U+0031 "1" */
    0x1, 0x80, 0xc0, 0xe0, 0xe1, 0xf7, 0xfb, 0xfc,
    0xc, 0xe, 0x7, 0x3, 0x81, 0xc0, 0xc0, 0xe0,
    0x70, 0x38, 0x1c, 0x1c, 0xe, 0x7, 0x3, 0x81,
    0x81, 0xc0,

    /* U+0032 "2" */
    0x0, 0x7f, 0x0, 0x3f, 0xf0, 0xf, 0xf, 0x3,
    0x80, 0xf0, 0x70, 0xe, 0x1c, 0x1, 0xc3, 0x80,
    0x38, 0x0, 0xe, 0x0, 0x1, 0xc0, 0x0, 0x70,
    0x0, 0x3c, 0x0, 0x1f, 0x0, 0xf, 0x80, 0x7,
    0xc0, 0x1, 0xe0, 0x0, 0xf0, 0x0, 0x1c, 0x0,
    0x7, 0x0, 0x1, 0xc0, 0x0, 0x3f, 0xff, 0x87,
    0xff, 0xe1, 0xff, 0xfc, 0x0,

    /* U+0033 "3" */
    0x0, 0xfc, 0x1, 0xff, 0x81, 0xe1, 0xe1, 0xc0,
    0x70, 0xc0, 0x38, 0xe0, 0x1c, 0x70, 0xe, 0x0,
    0xe, 0x0, 0xe, 0x0, 0x7e, 0x0, 0x7f, 0x0,
    0x3, 0xc0, 0x0, 0xf0, 0x0, 0x38, 0x0, 0x1d,
    0xc0, 0xe, 0xe0, 0x6, 0x70, 0x7, 0x38, 0x7,
    0xe, 0x7, 0x87, 0xff, 0x81, 0xff, 0x0, 0x8,
    0x0,

    /* U+0034 "4" */
    0x0, 0x3, 0x0, 0x7, 0x0, 0xf, 0x0, 0x1e,
    0x0, 0x3e, 0x0, 0x7e, 0x0, 0xee, 0x1, 0xce,
    0x3, 0x8c, 0x7, 0x1c, 0xe, 0x1c, 0x1c, 0x1c,
    0x38, 0x18, 0x70, 0x38, 0x70, 0x38, 0xff, 0xff,
    0xff, 0xff, 0x0, 0x70, 0x0, 0x70, 0x0, 0x70,
    0x0, 0x70, 0x0, 0x70, 0x0, 0x60,

    /* U+0035 "5" */
    0x3, 0xff, 0xc0, 0xff, 0xf0, 0x7f, 0xfc, 0x1c,
    0x0, 0x6, 0x0, 0x3, 0x80, 0x0, 0xe0, 0x0,
    0x30, 0x0, 0x1d, 0xf8, 0x7, 0xff, 0x81, 0xe1,
    0xe0, 0xe0, 0x1c, 0x0, 0x7, 0x0, 0x1, 0xc0,
    0x0, 0x70, 0x0, 0x1c, 0x0, 0x7, 0x38, 0x1,
    0x8e, 0x0, 0xe3, 0x80, 0x70, 0x70, 0x38, 0x1f,
    0xfc, 0x3, 0xfe, 0x0, 0x8, 0x0,

    /* U+0036 "6" */
    0x0, 0xfc, 0x1, 0xff, 0x1, 0xc3, 0xc1, 0xc0,
    0xe1, 0xc0, 0x38, 0xc0, 0x0, 0xe0, 0x0, 0x60,
    0x0, 0x71, 0xf0, 0x3b, 0xfe, 0x1f, 0x7, 0xf,
    0x1, 0xcf, 0x0, 0xe7, 0x0, 0x73, 0x80, 0x39,
    0xc0, 0x1c, 0xe0, 0xc, 0x70, 0xe, 0x38, 0x6,
    0xe, 0x7, 0x7, 0xff, 0x1, 0xfe, 0x0, 0x18,
    0x0,

    /* U+0037 "7" */
    0x3f, 0xff, 0x9f, 0xff, 0xcf, 0xff, 0xe0, 0x0,
    0xe0, 0x0, 0x60, 0x0, 0x60, 0x0, 0x60, 0x0,
    0x60, 0x0, 0x70, 0x0, 0x70, 0x0, 0x70, 0x0,
    0x30, 0x0, 0x38, 0x0, 0x38, 0x0, 0x38, 0x0,
    0x1c, 0x0, 0x1c, 0x0, 0xe, 0x0, 0xe, 0x0,
    0x7, 0x0, 0x7, 0x0, 0x3, 0x80, 0x0,

    /* U+0038 "8" */
    0x0, 0xfc, 0x1, 0xff, 0x81, 0xe1, 0xe1, 0xc0,
    0x70, 0xe0, 0x18, 0x60, 0x1c, 0x70, 0xe, 0x38,
    0xf, 0xe, 0xf, 0x3, 0xff, 0x3, 0xff, 0x3,
    0xc3, 0xc3, 0x80, 0xe3, 0x80, 0x39, 0xc0, 0x1c,
    0xc0, 0xe, 0xe0, 0x7, 0x70, 0x7, 0x1c, 0x3,
    0x8e, 0x3, 0x83, 0xff, 0x80, 0xff, 0x0, 0xc,
    0x0,

    /* U+0039 "9" */
    0x1, 0xf8, 0x7, 0xfc, 0xe, 0x1e, 0x1c, 0x7,
    0x38, 0x7, 0x30, 0x7, 0x70, 0x7, 0x70, 0x7,
    0x70, 0x7, 0x70, 0xf, 0x70, 0x1f, 0x38, 0x3e,
    0x3f, 0xfe, 0xf, 0xce, 0x0, 0xe, 0x0, 0x1c,
    0x0, 0x1c, 0xe0, 0x38, 0xe0, 0x38, 0xe0, 0x70,
    0x7f, 0xe0, 0x3f, 0x80, 0x4, 0x0,

    /* U+003C "<" */
    0x0, 0x0, 0x80, 0x1, 0xc0, 0x7, 0xe0, 0x1f,
    0x80, 0x3f, 0x0, 0xfc, 0x3, 0xf0, 0x1, 0xe0,
    0x0, 0xf8, 0x0, 0x3f, 0x0, 0x3, 0xe0, 0x0,
    0x7e, 0x0, 0xf, 0xc0, 0x1, 0xe0, 0x0, 0x30,

    /* U+003E ">" */
    0x10, 0x0, 0x1e, 0x0, 0xf, 0xc0, 0x1, 0xf8,
    0x0, 0x1f, 0x80, 0x3, 0xf0, 0x0, 0x7e, 0x0,
    0xf, 0x0, 0x1f, 0x80, 0x3f, 0x0, 0xfc, 0x3,
    0xf0, 0x7, 0xe0, 0x7, 0x80, 0x3, 0x0, 0x0,

    /* U+003F "?" */
    0x7, 0xe0, 0x7f, 0xe3, 0x83, 0xdc, 0x7, 0x60,
    0x1f, 0x80, 0x7e, 0x1, 0xc0, 0xf, 0x0, 0x78,
    0x3, 0xc0, 0x1e, 0x0, 0xf0, 0x7, 0x0, 0x38,
    0x0, 0xe0, 0x7, 0x0, 0x1c, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x38, 0x0, 0xe0, 0x3, 0x80,
    0x0,

    /* U+0041 "A" */
    0x0, 0x7, 0x80, 0x0, 0xf8, 0x0, 0x1f, 0x80,
    0x1, 0xf8, 0x0, 0x3b, 0x80, 0x3, 0xb8, 0x0,
    0x71, 0xc0, 0xf, 0x1c, 0x0, 0xe1, 0xc0, 0x1e,
    0x1c, 0x1, 0xc1, 0xc0, 0x38, 0x1c, 0x3, 0x81,
    0xc0, 0x70, 0x1e, 0xf, 0xff, 0xe0, 0xff, 0xfe,
    0x1c, 0x0, 0xe1, 0xc0, 0xe, 0x38, 0x0, 0xe3,
    0x80, 0xe, 0x70, 0x0, 0xff, 0x0, 0xf, 0xe0,
    0x0, 0x70,

    /* U+0042 "B" */
    0xf, 0xff, 0x80, 0xff, 0xfe, 0xe, 0x1, 0xe0,
    0xe0, 0xf, 0xc, 0x0, 0x71, 0xc0, 0x7, 0x1c,
    0x0, 0xe1, 0xc0, 0xe, 0x1c, 0x1, 0xc3, 0x80,
    0x38, 0x3f, 0xff, 0x3, 0xff, 0xf8, 0x38, 0x3,
    0xc3, 0x80, 0x1c, 0x70, 0x1, 0xc7, 0x0, 0x1c,
    0x70, 0x1, 0xc7, 0x0, 0x1c, 0xf0, 0x1, 0xce,
    0x0, 0x38, 0xe0, 0x7, 0xf, 0xff, 0xe0, 0xff,
    0xf8, 0x0,

    /* U+0043 "C" */
    0x0, 0x3f, 0x0, 0xf, 0xfe, 0x0, 0xf0, 0x78,
    0x1e, 0x1, 0xe0, 0xe0, 0x7, 0xe, 0x0, 0x38,
    0xe0, 0x1, 0xe7, 0x0, 0x0, 0x70, 0x0, 0x3,
    0x80, 0x0, 0x38, 0x0, 0x1, 0xc0, 0x0, 0xe,
    0x0, 0x0, 0x70, 0x0, 0x3, 0x80, 0x0, 0x1c,
    0x0, 0x1c, 0xe0, 0x0, 0xe7, 0x0, 0xe, 0x38,
    0x0, 0xe0, 0xe0, 0xf, 0x7, 0x80, 0xf0, 0x1f,
    0xff, 0x0, 0x7f, 0xe0, 0x0, 0x20, 0x0,

    /* U+0044 "D" */
    0x7, 0xff, 0x80, 0x7f, 0xff, 0x3, 0x80, 0x7c,
    0x1c, 0x0, 0xe0, 0xe0, 0x7, 0x87, 0x0, 0x1c,
    0x70, 0x0, 0xe3, 0x80, 0x7, 0x1c, 0x0, 0x38,
    0xe0, 0x1, 0xc7, 0x0, 0xe, 0x70, 0x0, 0x73,
    0x80, 0x3, 0x9c, 0x0, 0x38, 0xe0, 0x1, 0xcf,
    0x0, 0xe, 0x70, 0x0, 0xe3, 0x80, 0xe, 0x1c,
    0x0, 0x70, 0xe0, 0x7, 0xf, 0x0, 0xf0, 0x7f,
    0xff, 0x3, 0xff, 0xc0, 0x0,

    /* U+0045 "E" */
    0x7, 0xff, 0xf8, 0x3f, 0xff, 0xc3, 0x80, 0x0,
    0x1c, 0x0, 0x0, 0xe0, 0x0, 0x7, 0x0, 0x0,
    0x70, 0x0, 0x3, 0x80, 0x0, 0x1c, 0x0, 0x0,
    0xe0, 0x0, 0x7, 0xff, 0xf0, 0x7f, 0xff, 0x83,
    0x80, 0x0, 0x1c, 0x0, 0x0, 0xe0, 0x0, 0xf,
    0x0, 0x0, 0x70, 0x0, 0x3, 0x80, 0x0, 0x1c,
    0x0, 0x0, 0xe0, 0x0, 0xf, 0x0, 0x0, 0x7f,
    0xff, 0xc3, 0xff, 0xfe, 0x0,

    /* U+0046 "F" */
    0x7, 0xff, 0xf0, 0x7f, 0xff, 0xe, 0x0, 0x0,
    0xe0, 0x0, 0xe, 0x0, 0x0, 0xe0, 0x0, 0x1c,
    0x0, 0x1, 0xc0, 0x0, 0x1c, 0x0, 0x1, 0xc0,
    0x0, 0x1f, 0xff, 0x83, 0xff, 0xf0, 0x38, 0x0,
    0x3, 0x80, 0x0, 0x38, 0x0, 0x7, 0x0, 0x0,
    0x70, 0x0, 0x7, 0x0, 0x0, 0x70, 0x0, 0x7,
    0x0, 0x0, 0xe0, 0x0, 0xe, 0x0, 0x0, 0xe0,
    0x0, 0x0,

    /* U+0047 "G" */
    0x0, 0x1f, 0xc0, 0x1, 0xff, 0xe0, 0x7, 0x81,
    0xf0, 0x1c, 0x0, 0xe0, 0x70, 0x1, 0xe1, 0xc0,
    0x1, 0xc7, 0x0, 0x3, 0x9c, 0x0, 0x0, 0x38,
    0x0, 0x0, 0xe0, 0x0, 0x1, 0xc0, 0x0, 0x3,
    0x80, 0x7f, 0xe7, 0x0, 0xff, 0xce, 0x0, 0x3,
    0xbc, 0x0, 0x7, 0x78, 0x0, 0xc, 0x70, 0x0,
    0x38, 0xe0, 0x0, 0x71, 0xc0, 0x1, 0xe1, 0xc0,
    0x7, 0xc3, 0xc0, 0x3f, 0x3, 0xff, 0xe6, 0x1,
    0xff, 0xc, 0x0, 0x60, 0x0,

    /* U+0048 "H" */
    0xf, 0x0, 0x1c, 0x1c, 0x0, 0x38, 0x38, 0x0,
    0x70, 0x70, 0x0, 0xe0, 0xe0, 0x1, 0xc3, 0x80,
    0x7, 0x7, 0x0, 0xe, 0xe, 0x0, 0x1c, 0x1c,
    0x0, 0x38, 0x38, 0x0, 0x70, 0xff, 0xff, 0xc1,
    0xff, 0xff, 0x83, 0x80, 0x7, 0x7, 0x0, 0xe,
    0x1c, 0x0, 0x38, 0x38, 0x0, 0x70, 0x70, 0x0,
    0xe0, 0xe0, 0x1, 0xc1, 0xc0, 0x3, 0x87, 0x0,
    0xe, 0xe, 0x0, 0x1c, 0x1c, 0x0, 0x38, 0x38,
    0x0, 0x70, 0x0,

    /* U+0049 "I" */
    0x7, 0x7, 0x7, 0xe, 0xe, 0xe, 0xe, 0xe,
    0x1c, 0x1c, 0x1c, 0x1c, 0x38, 0x38, 0x38, 0x38,
    0x38, 0x70, 0x70, 0x70, 0x70, 0x70, 0xe0,

    /* U+004A "J" */
    0x0, 0x3, 0x80, 0x0, 0xe0, 0x0, 0x38, 0x0,
    0xe, 0x0, 0x3, 0x80, 0x1, 0xc0, 0x0, 0x70,
    0x0, 0x1c, 0x0, 0x7, 0x0, 0x1, 0xc0, 0x0,
    0xe0, 0x0, 0x38, 0x0, 0xe, 0x0, 0x3, 0x80,
    0x1, 0xc0, 0x0, 0x70, 0x70, 0x1c, 0x1c, 0x7,
    0x7, 0x3, 0x83, 0xc0, 0xe0, 0x70, 0x70, 0x1f,
    0xfc, 0x3, 0xfc, 0x0, 0x10, 0x0,

    /* U+004B "K" */
    0xe, 0x0, 0x1c, 0x1c, 0x0, 0xf0, 0x38, 0x3,
    0xc0, 0x70, 0xf, 0x0, 0xe0, 0x3c, 0x3, 0x80,
    0xe0, 0x7, 0x7, 0x80, 0xe, 0x1e, 0x0, 0x1c,
    0x78, 0x0, 0x71, 0xe0, 0x0, 0xef, 0xc0, 0x1,
    0xff, 0x80, 0x3, 0xf3, 0x80, 0x7, 0x87, 0x0,
    0x1e, 0x7, 0x0, 0x38, 0xe, 0x0, 0x70, 0xe,
    0x0, 0xe0, 0x1c, 0x3, 0xc0, 0x3c, 0x7, 0x0,
    0x38, 0xe, 0x0, 0x78, 0x1c, 0x0, 0x70, 0x38,
    0x0, 0xf0, 0x0,

    /* U+004C "L" */
    0xe, 0x0, 0x1c, 0x0, 0x38, 0x0, 0x70, 0x0,
    0xe0, 0x3, 0x80, 0x7, 0x0, 0xe, 0x0, 0x1c,
    0x0, 0x78, 0x0, 0xe0, 0x1, 0xc0, 0x3, 0x80,
    0x7, 0x0, 0x1c, 0x0, 0x38, 0x0, 0x70, 0x0,
    0xe0, 0x1, 0xc0, 0x7, 0x0, 0xe, 0x0, 0x1f,
    0xff, 0xbf, 0xff, 0x0,

    /* U+004D "M" */
    0x7, 0xc0, 0x1, 0xe0, 0xf8, 0x0, 0x7c, 0x1f,
    0x0, 0xf, 0x83, 0xe0, 0x3, 0xf0, 0xfc, 0x0,
    0x7c, 0x1d, 0x80, 0x1f, 0x83, 0xb0, 0x7, 0x70,
    0x76, 0x0, 0xee, 0xc, 0xe0, 0x39, 0x83, 0x9c,
    0x7, 0x70, 0x73, 0x81, 0xce, 0xe, 0x70, 0x31,
    0xc1, 0xce, 0xe, 0x38, 0x70, 0xc1, 0x86, 0xe,
    0x18, 0x71, 0xc1, 0xc3, 0x1c, 0x38, 0x38, 0x73,
    0x87, 0x6, 0xe, 0xe0, 0xe1, 0xc1, 0xd8, 0x38,
    0x38, 0x3f, 0x7, 0x7, 0x3, 0xc0, 0xe0, 0xe0,
    0x78, 0x1c, 0x18, 0xe, 0x3, 0x0,

    /* U+004E "N" */
    0x7, 0x80, 0x7, 0x7, 0x80, 0xe, 0x7, 0xc0,
    0xe, 0x7, 0xc0, 0xe, 0xf, 0xe0, 0xe, 0xe,
    0xe0, 0xc, 0xe, 0xe0, 0x1c, 0xe, 0x70, 0x1c,
    0xc, 0x70, 0x1c, 0x1c, 0x38, 0x1c, 0x1c, 0x38,
    0x18, 0x1c, 0x3c, 0x38, 0x1c, 0x1c, 0x38, 0x38,
    0x1c, 0x38, 0x38, 0xe, 0x38, 0x38, 0xe, 0x70,
    0x38, 0xf, 0x70, 0x30, 0x7, 0x70, 0x70, 0x7,
    0xf0, 0x70, 0x3, 0xe0, 0x70, 0x3, 0xe0, 0x70,
    0x1, 0xe0, 0x60, 0x1, 0xe0,

    /* U+004F "O" */
    0x0, 0x1f, 0xc0, 0x0, 0xff, 0xf0, 0x1, 0xe0,
    0xf8, 0x7, 0x80, 0x3c, 0xf, 0x0, 0x1e, 0x1e,
    0x0, 0xe, 0x1c, 0x0, 0xe, 0x38, 0x0, 0xe,
    0x38, 0x0, 0xe, 0x70, 0x0, 0xe, 0x70, 0x0,
    0xe, 0x70, 0x0, 0xe, 0xf0, 0x0, 0xe, 0xe0,
    0x0, 0xe, 0xe0, 0x0, 0x1c, 0xe0, 0x0, 0x1c,
    0xf0, 0x0, 0x38, 0x70, 0x0, 0x38, 0x70, 0x0,
    0x70, 0x78, 0x0, 0xe0, 0x3c, 0x3, 0xc0, 0x1f,
    0xff, 0x80, 0x7, 0xfe, 0x0, 0x0, 0x40, 0x0,

    /* U+0050 "P" */
    0x7, 0xff, 0xc0, 0x7f, 0xfe, 0xe, 0x0, 0xf0,
    0xe0, 0x7, 0xe, 0x0, 0x70, 0xe0, 0x7, 0x1c,
    0x0, 0x71, 0xc0, 0x7, 0x1c, 0x0, 0x71, 0xc0,
    0xe, 0x1c, 0x3, 0xc3, 0xff, 0xf8, 0x3f, 0xfe,
    0x3, 0x80, 0x0, 0x38, 0x0, 0x3, 0x80, 0x0,
    0x70, 0x0, 0x7, 0x0, 0x0, 0x70, 0x0, 0x7,
    0x0, 0x0, 0xe0, 0x0, 0xe, 0x0, 0x0, 0xe0,
    0x0, 0x0,

    /* U+0051 "Q" */
    0x0, 0x1f, 0xc0, 0x0, 0xff, 0xf0, 0x3, 0xe0,
    0xf8, 0x7, 0x80, 0x3c, 0xf, 0x0, 0x1e, 0x1e,
    0x0, 0xe, 0x1c, 0x0, 0xe, 0x38, 0x0, 0xe,
    0x38, 0x0, 0xe, 0x70, 0x0, 0xe, 0x70, 0x0,
    0xe, 0x70, 0x0, 0xe, 0xf0, 0x0, 0xe, 0xe0,
    0x0, 0xe, 0xe0, 0x0, 0x1c, 0xe0, 0x0, 0x1c,
    0xf0, 0x0, 0x3c, 0x70, 0x6, 0x38, 0x70, 0xf,
    0x70, 0x78, 0x7, 0xe0, 0x3c, 0x3, 0xc0, 0x1f,
    0xff, 0xe0, 0x7, 0xfe, 0xf0, 0x0, 0x40, 0x70,
    0x0, 0x0, 0x20,

    /* U+0052 "R" */
    0x7, 0xff, 0xe0, 0x1f, 0xff, 0xe0, 0xe0, 0x7,
    0x83, 0x80, 0xe, 0xe, 0x0, 0x1c, 0x38, 0x0,
    0x70, 0xe0, 0x3, 0x87, 0x0, 0xe, 0x1c, 0x0,
    0x38, 0x70, 0x1, 0xc1, 0xc0, 0xe, 0xf, 0xff,
    0xf0, 0x3f, 0xff, 0xc0, 0xe0, 0x7, 0x83, 0x80,
    0xe, 0xe, 0x0, 0x38, 0x70, 0x0, 0xe1, 0xc0,
    0x3, 0x87, 0x0, 0x1e, 0x1c, 0x0, 0x70, 0xf0,
    0x1, 0xc3, 0x80, 0x7, 0xe, 0x0, 0x1e, 0x0,

    /* U+0053 "S" */
    0x0, 0x7f, 0x0, 0x3f, 0xfc, 0x7, 0x81, 0xe0,
    0xe0, 0xf, 0xc, 0x0, 0x71, 0xc0, 0x7, 0x1c,
    0x0, 0x71, 0xc0, 0x0, 0x1e, 0x0, 0x1, 0xfc,
    0x0, 0xf, 0xfc, 0x0, 0x7f, 0xf0, 0x0, 0xff,
    0x80, 0x1, 0xfc, 0x0, 0x3, 0xc0, 0x0, 0x1c,
    0xe0, 0x1, 0xce, 0x0, 0x1c, 0xe0, 0x1, 0x8e,
    0x0, 0x38, 0x70, 0x7, 0x7, 0xff, 0xe0, 0x1f,
    0xf8, 0x0, 0x10, 0x0,

    /* U+0054 "T" */
    0xff, 0xff, 0xff, 0xff, 0xfc, 0x3, 0x80, 0x0,
    0xe0, 0x0, 0x1c, 0x0, 0x3, 0x80, 0x0, 0x70,
    0x0, 0xe, 0x0, 0x3, 0x80, 0x0, 0x70, 0x0,
    0xe, 0x0, 0x1, 0xc0, 0x0, 0x38, 0x0, 0xe,
    0x0, 0x1, 0xc0, 0x0, 0x38, 0x0, 0x7, 0x0,
    0x0, 0xe0, 0x0, 0x38, 0x0, 0x7, 0x0, 0x0,
    0xe0, 0x0, 0x1c, 0x0, 0x7, 0x0, 0x0,

    /* U+0055 "U" */
    0xe, 0x0, 0x38, 0x70, 0x0, 0xe1, 0xc0, 0x3,
    0x87, 0x0, 0xe, 0x1c, 0x0, 0x38, 0xf0, 0x1,
    0xc3, 0x80, 0x7, 0xe, 0x0, 0x1c, 0x38, 0x0,
    0x70, 0xe0, 0x1, 0xc7, 0x0, 0xe, 0x1c, 0x0,
    0x38, 0x70, 0x0, 0xe1, 0xc0, 0x3, 0x87, 0x0,
    0x1c, 0x38, 0x0, 0x70, 0xe0, 0x1, 0xc3, 0x80,
    0x7, 0xe, 0x0, 0x38, 0x3c, 0x1, 0xe0, 0x70,
    0xf, 0x0, 0xff, 0xf0, 0x1, 0xff, 0x80, 0x0,
    0x40, 0x0,

    /* U+0056 "V" */
    0xe0, 0x0, 0xee, 0x0, 0xe, 0xe0, 0x1, 0xcf,
    0x0, 0x3c, 0x70, 0x3, 0x87, 0x0, 0x70, 0x70,
    0x7, 0x7, 0x0, 0xe0, 0x70, 0xe, 0x7, 0x1,
    0xc0, 0x70, 0x1c, 0x3, 0x83, 0x80, 0x38, 0x38,
    0x3, 0x87, 0x0, 0x38, 0xe0, 0x3, 0x8e, 0x0,
    0x39, 0xc0, 0x1, 0x9c, 0x0, 0x1f, 0x80, 0x1,
    0xf8, 0x0, 0x1f, 0x0, 0x1, 0xe0, 0x0, 0x1e,
    0x0, 0x0,

    /* U+0057 "W" */
    0xe0, 0xf, 0x0, 0x77, 0x0, 0x78, 0x3, 0xb8,
    0x7, 0xc0, 0x3d, 0xc0, 0x3e, 0x1, 0xce, 0x3,
    0xf0, 0x1e, 0x70, 0x1f, 0x80, 0xe3, 0x81, 0xdc,
    0xf, 0x1c, 0xe, 0xe0, 0x70, 0xe0, 0xe3, 0x3,
    0x87, 0x7, 0x18, 0x38, 0x38, 0x70, 0xc1, 0xc1,
    0xc3, 0x86, 0x1c, 0xe, 0x38, 0x30, 0xe0, 0x71,
    0xc1, 0xce, 0x3, 0x9c, 0xe, 0x70, 0x1c, 0xe0,
    0x77, 0x0, 0xee, 0x3, 0xb8, 0x7, 0x70, 0x1f,
    0x80, 0x3f, 0x0, 0xfc, 0x0, 0xf8, 0x7, 0xc0,
    0x7, 0x80, 0x3e, 0x0, 0x3c, 0x1, 0xf0, 0x1,
    0xc0, 0x7, 0x0, 0x0,

    /* U+0058 "X" */
    0x7, 0x80, 0xf, 0x3, 0x80, 0x1e, 0x3, 0xc0,
    0x1c, 0x1, 0xc0, 0x38, 0x1, 0xe0, 0x70, 0x0,
    0xe0, 0xe0, 0x0, 0xf1, 0xc0, 0x0, 0x73, 0x80,
    0x0, 0x7f, 0x80, 0x0, 0x3f, 0x0, 0x0, 0x3e,
    0x0, 0x0, 0x3c, 0x0, 0x0, 0x7e, 0x0, 0x0,
    0xfe, 0x0, 0x1, 0xef, 0x0, 0x1, 0xc7, 0x0,
    0x3, 0x87, 0x80, 0x7, 0x3, 0x80, 0xe, 0x3,
    0xc0, 0x1c, 0x1, 0xc0, 0x3c, 0x1, 0xe0, 0x78,
    0x1, 0xe0, 0xf0, 0x0, 0xf0,

    /* U+0059 "Y" */
    0x70, 0x0, 0x7b, 0xc0, 0x7, 0x9e, 0x0, 0x78,
    0x70, 0x3, 0x83, 0xc0, 0x38, 0xe, 0x3, 0x80,
    0x70, 0x3c, 0x1, 0xc3, 0xc0, 0xe, 0x1c, 0x0,
    0x79, 0xc0, 0x1, 0xdc, 0x0, 0xf, 0xc0, 0x0,
    0x3e, 0x0, 0x1, 0xe0, 0x0, 0xe, 0x0, 0x0,
    0x70, 0x0, 0x3, 0x80, 0x0, 0x1c, 0x0, 0x1,
    0xc0, 0x0, 0xe, 0x0, 0x0, 0x70, 0x0, 0x3,
    0x80, 0x0, 0x3c, 0x0, 0x0,

    /* U+005A "Z" */
    0x3, 0xff, 0xfc, 0xf, 0xff, 0xf8, 0x0, 0x0,
    0xf0, 0x0, 0x3, 0xc0, 0x0, 0xf, 0x0, 0x0,
    0x3c, 0x0, 0x0, 0xf0, 0x0, 0x3, 0xc0, 0x0,
    0xf, 0x0, 0x0, 0x3c, 0x0, 0x0, 0xf0, 0x0,
    0x3, 0xc0, 0x0, 0xf, 0x80, 0x0, 0x3e, 0x0,
    0x0, 0xf8, 0x0, 0x3, 0xe0, 0x0, 0xf, 0x80,
    0x0, 0x3e, 0x0, 0x0, 0xf8, 0x0, 0x3, 0xe0,
    0x0, 0xf, 0x80, 0x0, 0x1f, 0xff, 0xf8, 0x3f,
    0xff, 0xf0, 0x0,

    /* U+005B "[" */
    0x3, 0xf0, 0x3f, 0x7, 0x0, 0x70, 0x7, 0x0,
    0x60, 0x6, 0x0, 0xe0, 0xe, 0x0, 0xe0, 0xc,
    0x0, 0xc0, 0x1c, 0x1, 0xc0, 0x1c, 0x1, 0x80,
    0x38, 0x3, 0x80, 0x38, 0x3, 0x80, 0x30, 0x7,
    0x0, 0x70, 0x7, 0x0, 0x70, 0x6, 0x0, 0xe0,
    0xe, 0x0, 0xfc, 0xf, 0xc0,

    /* U+005D "]" */
    0x1, 0xf8, 0x1f, 0x80, 0x1c, 0x0, 0xe0, 0x7,
    0x0, 0x30, 0x1, 0x80, 0x1c, 0x0, 0xe0, 0x7,
    0x0, 0x30, 0x3, 0x80, 0x1c, 0x0, 0xe0, 0x7,
    0x0, 0x30, 0x3, 0x80, 0x1c, 0x0, 0xe0, 0x6,
    0x0, 0x30, 0x3, 0x80, 0x1c, 0x0, 0xe0, 0x6,
    0x0, 0x30, 0x3, 0x80, 0x1c, 0x7, 0xe0, 0x7e,
    0x0,

    /* U+0061 "a" */
    0x1, 0xfc, 0x7, 0xfe, 0xf, 0xf, 0x1c, 0x7,
    0x1c, 0x7, 0x0, 0x7, 0x0, 0xf, 0x3, 0xfe,
    0x1f, 0xfe, 0x3f, 0xce, 0x78, 0xe, 0xf0, 0xe,
    0xe0, 0xc, 0xe0, 0x1c, 0xe0, 0x7c, 0x7f, 0xff,
    0x7f, 0x9e, 0x8, 0x4,

    /* U+0062 "b" */
    0x6, 0x0, 0x7, 0x0, 0x3, 0x80, 0x1, 0xc0,
    0x0, 0xc0, 0x0, 0xe0, 0x0, 0x73, 0xf0, 0x3f,
    0xfc, 0x1f, 0xf, 0xf, 0x3, 0x8f, 0x0, 0xe7,
    0x0, 0x73, 0x80, 0x39, 0x80, 0x1c, 0xc0, 0xe,
    0xe0, 0x6, 0x70, 0x7, 0x38, 0x3, 0x9c, 0x3,
    0x9e, 0x1, 0xcf, 0x81, 0xc6, 0xff, 0xc3, 0x3f,
    0x80, 0x2, 0x0,

    /* U+0063 "c" */
    0x1, 0xf8, 0x7, 0xfc, 0xf, 0xe, 0x1c, 0x7,
    0x38, 0x7, 0x38, 0x7, 0x70, 0x0, 0x70, 0x0,
    0x70, 0x0, 0x60, 0x0, 0xe0, 0x0, 0xe0, 0xe,
    0x60, 0x1c, 0x70, 0x1c, 0x70, 0x38, 0x3f, 0xf0,
    0x1f, 0xe0, 0x2, 0x0,

    /* U+0064 "d" */
    0x0, 0x0, 0xe0, 0x0, 0x18, 0x0, 0x3, 0x0,
    0x0, 0xe0, 0x0, 0x1c, 0x0, 0x3, 0x80, 0x7c,
    0x60, 0x3f, 0xfc, 0xe, 0x1f, 0x83, 0x81, 0xf0,
    0xe0, 0x1e, 0x18, 0x3, 0x87, 0x0, 0x70, 0xe0,
    0xe, 0x18, 0x1, 0xc7, 0x0, 0x30, 0xe0, 0xe,
    0x1c, 0x1, 0xc3, 0x80, 0x38, 0x38, 0xf, 0x7,
    0x3, 0xc0, 0x7f, 0xd8, 0x7, 0xf7, 0x0, 0x20,
    0x0,

    /* U+0065 "e" */
    0x3, 0xf0, 0xf, 0xfc, 0x1e, 0x1e, 0x38, 0xe,
    0x30, 0x7, 0x70, 0x7, 0x60, 0x6, 0xff, 0xfe,
    0xff, 0xfe, 0xe0, 0x0, 0xe0, 0x0, 0xe0, 0x0,
    0xe0, 0x1c, 0xe0, 0x38, 0xf0, 0x78, 0x7f, 0xf0,
    0x3f, 0xc0, 0x4, 0x0,

    /* U+0066 "f" */
    0x3, 0xc1, 0xf0, 0xe0, 0x38, 0xc, 0x3, 0x7,
    0xf9, 0xfe, 0x1c, 0x6, 0x3, 0x80, 0xe0, 0x38,
    0xe, 0x3, 0x1, 0xc0, 0x70, 0x1c, 0x6, 0x1,
    0x80, 0xe0, 0x38, 0xe, 0x0,

    /* U+0067 "g" */
    0x0, 0xf8, 0xc0, 0xff, 0xf0, 0x70, 0xfc, 0x38,
    0x1f, 0x1c, 0x3, 0x87, 0x0, 0xe3, 0x80, 0x38,
    0xe0, 0xe, 0x38, 0x3, 0xc, 0x0, 0xc7, 0x0,
    0x71, 0xc0, 0x1c, 0x70, 0xf, 0xe, 0x3, 0x83,
    0x81, 0xe0, 0x7f, 0xf8, 0xf, 0xee, 0x0, 0x83,
    0x0, 0x1, 0xc3, 0x80, 0x70, 0xe0, 0x38, 0x3c,
    0x1c, 0x7, 0xfe, 0x0, 0x7e, 0x0,

    /* U+0068 "h" */
    0x7, 0x0, 0x7, 0x0, 0x6, 0x0, 0xe, 0x0,
    0xe, 0x0, 0xe, 0x0, 0xc, 0x7c, 0xd, 0xff,
    0x1f, 0x8f, 0x1e, 0x7, 0x1c, 0x3, 0x1c, 0x7,
    0x38, 0x7, 0x38, 0x7, 0x38, 0x7, 0x38, 0x6,
    0x30, 0xe, 0x70, 0xe, 0x70, 0xe, 0x70, 0xc,
    0x60, 0xc, 0x60, 0x1c, 0xe0, 0x1c,

    /* U+0069 "i" */
    0x7, 0x6, 0xe, 0x0, 0x0, 0x0, 0xc, 0x1c,
    0x1c, 0x1c, 0x18, 0x38, 0x38, 0x38, 0x38, 0x30,
    0x70, 0x70, 0x70, 0x70, 0x60, 0xe0, 0xe0,

    /* U+006A "j" */
    0x0, 0x70, 0x7, 0x0, 0x60, 0x0, 0x0, 0x0,
    0x0, 0x0, 0xc0, 0x1c, 0x1, 0xc0, 0x1c, 0x1,
    0xc0, 0x18, 0x3, 0x80, 0x38, 0x3, 0x80, 0x30,
    0x3, 0x0, 0x70, 0x7, 0x0, 0x70, 0x6, 0x0,
    0x60, 0xe, 0x0, 0xe0, 0xe, 0x0, 0xc0, 0x1c,
    0x1, 0xc0, 0xf8, 0xf, 0x0,

    /* U+006B "k" */
    0x6, 0x0, 0x7, 0x0, 0x3, 0x80, 0x1, 0xc0,
    0x0, 0xe0, 0x0, 0x60, 0x0, 0x70, 0x1c, 0x38,
    0x3c, 0x1c, 0x3c, 0xc, 0x38, 0x6, 0x38, 0x7,
    0x38, 0x3, 0xfc, 0x1, 0xfe, 0x0, 0xf3, 0x80,
    0xf1, 0xc0, 0x70, 0xe0, 0x38, 0x38, 0x1c, 0x1c,
    0xc, 0x7, 0xe, 0x3, 0x87, 0x1, 0xc3, 0x80,
    0x70,

    /* U+006C "l" */
    0x7, 0x7, 0x6, 0xe, 0xe, 0xe, 0xc, 0xc,
    0x1c, 0x1c, 0x1c, 0x18, 0x38, 0x38, 0x38, 0x30,
    0x30, 0x70, 0x70, 0x70, 0x60, 0xe0, 0xe0,

    /* U+006D "m" */
    0xc, 0x7c, 0x3f, 0x6, 0xff, 0x3f, 0xc7, 0xc3,
    0xf0, 0xe3, 0xc0, 0xf0, 0x71, 0xc0, 0x70, 0x38,
    0xc0, 0x70, 0x1c, 0xe0, 0x38, 0xe, 0x70, 0x1c,
    0x7, 0x38, 0xe, 0x3, 0x1c, 0x6, 0x3, 0x8c,
    0x7, 0x1, 0xce, 0x3, 0x80, 0xe7, 0x1, 0xc0,
    0x73, 0x80, 0xe0, 0x31, 0x80, 0x60, 0x38, 0xc0,
    0x70, 0x1c, 0xe0, 0x38, 0xe, 0x0,

    /* U+006E "n" */
    0xc, 0x7c, 0x1d, 0xff, 0x1f, 0x8f, 0x1e, 0x7,
    0x1c, 0x3, 0x1c, 0x7, 0x38, 0x7, 0x38, 0x7,
    0x38, 0x7, 0x38, 0x6, 0x30, 0xe, 0x70, 0xe,
    0x70, 0xe, 0x70, 0xc, 0x60, 0xc, 0x60, 0x1c,
    0xe0, 0x1c,

    /* U+006F "o" */
    0x3, 0xf0, 0x1f, 0xf8, 0x78, 0x71, 0xc0, 0x73,
    0x0, 0xee, 0x1, 0xd8, 0x1, 0xf0, 0x3, 0xe0,
    0xf, 0xc0, 0x1f, 0x0, 0x3e, 0x0, 0xec, 0x1,
    0xdc, 0x7, 0x3c, 0x1c, 0x3f, 0xf0, 0x3f, 0xc0,
    0x8, 0x0,

    /* U+0070 "p" */
    0x7, 0x3f, 0x0, 0xff, 0xf0, 0x1f, 0xf, 0x3,
    0xc0, 0xe0, 0xf0, 0xe, 0x1c, 0x1, 0xc3, 0x80,
    0x38, 0x60, 0x7, 0x1c, 0x0, 0xe3, 0x80, 0x38,
    0x70, 0x7, 0xe, 0x0, 0xe1, 0xc0, 0x38, 0x78,
    0x7, 0xf, 0x81, 0xc1, 0xff, 0xf0, 0x3b, 0xf8,
    0x6, 0x8, 0x1, 0xc0, 0x0, 0x38, 0x0, 0x7,
    0x0, 0x0, 0xc0, 0x0, 0x18, 0x0, 0x7, 0x0,
    0x0,

    /* U+0071 "q" */
    0x1, 0xf1, 0x83, 0xfe, 0xc3, 0x87, 0xe3, 0x81,
    0xf3, 0x80, 0x71, 0xc0, 0x39, 0xc0, 0x1c, 0xe0,
    0xe, 0x60, 0x7, 0x70, 0x3, 0x38, 0x3, 0x9c,
    0x1, 0xce, 0x1, 0xe3, 0x80, 0xf1, 0xc0, 0xf0,
    0x7f, 0xf8, 0x1f, 0xdc, 0x2, 0xe, 0x0, 0x7,
    0x0, 0x3, 0x0, 0x3, 0x80, 0x1, 0xc0, 0x0,
    0xe0, 0x0, 0x70,

    /* U+0072 "r" */
    0xc, 0x71, 0xdf, 0x1f, 0xe1, 0xf8, 0x1e, 0x1,
    0xc0, 0x38, 0x3, 0x80, 0x38, 0x3, 0x80, 0x30,
    0x7, 0x0, 0x70, 0x7, 0x0, 0x60, 0x6, 0x0,
    0xe0, 0x0,

    /* U+0073 "s" */
    0x3, 0xf0, 0x1f, 0xf8, 0x70, 0x70, 0xc0, 0x73,
    0x80, 0xe7, 0x0, 0xf, 0x0, 0x1f, 0xe0, 0x1f,
    0xf0, 0xf, 0xf0, 0x1, 0xf0, 0x0, 0xee, 0x1,
    0xdc, 0x3, 0x38, 0xe, 0x7f, 0xf8, 0x3f, 0xe0,
    0x8, 0x0,

    /* U+0074 "t" */
    0xe, 0x7, 0x3, 0x3, 0x81, 0xc3, 0xfd, 0xfc,
    0x30, 0x38, 0x1c, 0xe, 0x7, 0x3, 0x3, 0x81,
    0xc0, 0xe0, 0x60, 0x70, 0x38, 0x1c, 0xf, 0x83,
    0xc0, 0xc0,

    /* U+0075 "u" */
    0x18, 0x7, 0x38, 0x7, 0x38, 0x7, 0x38, 0x6,
    0x30, 0xe, 0x70, 0xe, 0x70, 0xe, 0x70, 0xc,
    0x70, 0xc, 0x60, 0x1c, 0xe0, 0x1c, 0xe0, 0x1c,
    0xe0, 0x38, 0xe0, 0x38, 0xe0, 0x78, 0xff, 0xf8,
    0x7f, 0x38, 0x8, 0x0,

    /* U+0076 "v" */
    0xe0, 0xf, 0xc0, 0x3b, 0x80, 0x77, 0x1, 0xc6,
    0x3, 0x8c, 0xe, 0x1c, 0x18, 0x38, 0x70, 0x71,
    0xc0, 0xe3, 0x81, 0xce, 0x1, 0x98, 0x3, 0x70,
    0x6, 0xc0, 0xf, 0x80, 0x1e, 0x0, 0x38, 0x0,

    /* U+0077 "w" */
    0xe0, 0x78, 0x1f, 0x81, 0xe0, 0x7e, 0xf, 0x83,
    0xb8, 0x3e, 0xe, 0xe1, 0xf8, 0x73, 0x86, 0xe1,
    0xce, 0x3b, 0x8e, 0x38, 0xee, 0x38, 0xe7, 0x19,
    0xc1, 0x9c, 0x67, 0x6, 0x61, 0xb8, 0x1b, 0x86,
    0xe0, 0x6c, 0x1f, 0x1, 0xf0, 0x7c, 0x7, 0x81,
    0xe0, 0x1e, 0x7, 0x80, 0x70, 0x1c, 0x0,

    /* U+0078 "x" */
    0xe, 0x3, 0x83, 0x81, 0xc0, 0x70, 0xe0, 0x1c,
    0x38, 0x7, 0x1c, 0x0, 0xee, 0x0, 0x3f, 0x0,
    0x7, 0x80, 0x1, 0xc0, 0x0, 0xf8, 0x0, 0x7e,
    0x0, 0x39, 0xc0, 0x1e, 0x70, 0x7, 0xe, 0x3,
    0x83, 0x81, 0xc0, 0xe0, 0xe0, 0x1c, 0x0,

    /* U+0079 "y" */
    0xe, 0x0, 0xe1, 0xc0, 0x38, 0x18, 0x6, 0x3,
    0x1, 0xc0, 0x70, 0x30, 0xe, 0xe, 0x1, 0xc3,
    0x80, 0x38, 0x70, 0x7, 0x1c, 0x0, 0xe3, 0x0,
    0xc, 0xe0, 0x1, 0x98, 0x0, 0x37, 0x0, 0x7,
    0xc0, 0x0, 0xf8, 0x0, 0x1e, 0x0, 0x3, 0x80,
    0x0, 0x70, 0x0, 0xc, 0x0, 0x3, 0x80, 0x0,
    0x60, 0x0, 0x18, 0x0, 0x1f, 0x0, 0x7, 0x80,
    0x0,

    /* U+007A "z" */
    0xf, 0xff, 0x87, 0xff, 0xc0, 0x1, 0xc0, 0x1,
    0xc0, 0x1, 0xc0, 0x1, 0xc0, 0x1, 0xc0, 0x1,
    0xc0, 0x1, 0xc0, 0x1, 0xc0, 0x1, 0xc0, 0x1,
    0xc0, 0x1, 0xc0, 0x1, 0xc0, 0x1, 0xc0, 0x1,
    0xff, 0xf8, 0xff, 0xfc, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 142, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 299, .box_w = 16, .box_h = 16, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 142, .box_w = 5, .box_h = 8, .ofs_x = 2, .ofs_y = -5},
    {.bitmap_index = 38, .adv_w = 170, .box_w = 8, .box_h = 2, .ofs_x = 3, .ofs_y = 8},
    {.bitmap_index = 40, .adv_w = 142, .box_w = 4, .box_h = 3, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 42, .adv_w = 285, .box_w = 16, .box_h = 23, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 88, .adv_w = 285, .box_w = 9, .box_h = 23, .ofs_x = 7, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 285, .box_w = 19, .box_h = 22, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 285, .box_w = 17, .box_h = 23, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 216, .adv_w = 285, .box_w = 16, .box_h = 23, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 262, .adv_w = 285, .box_w = 18, .box_h = 24, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 316, .adv_w = 285, .box_w = 17, .box_h = 23, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 365, .adv_w = 285, .box_w = 17, .box_h = 22, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 412, .adv_w = 285, .box_w = 17, .box_h = 23, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 461, .adv_w = 285, .box_w = 16, .box_h = 23, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 507, .adv_w = 299, .box_w = 17, .box_h = 15, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 539, .adv_w = 299, .box_w = 17, .box_h = 15, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 571, .adv_w = 285, .box_w = 14, .box_h = 23, .ofs_x = 6, .ofs_y = 0},
    {.bitmap_index = 612, .adv_w = 342, .box_w = 20, .box_h = 23, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 670, .adv_w = 342, .box_w = 20, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 728, .adv_w = 370, .box_w = 21, .box_h = 24, .ofs_x = 4, .ofs_y = -1},
    {.bitmap_index = 791, .adv_w = 370, .box_w = 21, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 852, .adv_w = 342, .box_w = 21, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 913, .adv_w = 313, .box_w = 20, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 971, .adv_w = 398, .box_w = 23, .box_h = 24, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1040, .adv_w = 370, .box_w = 23, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1107, .adv_w = 142, .box_w = 8, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1130, .adv_w = 256, .box_w = 18, .box_h = 24, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1184, .adv_w = 342, .box_w = 23, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1251, .adv_w = 285, .box_w = 15, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1295, .adv_w = 426, .box_w = 27, .box_h = 23, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1373, .adv_w = 370, .box_w = 24, .box_h = 23, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1442, .adv_w = 398, .box_w = 24, .box_h = 24, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1514, .adv_w = 342, .box_w = 20, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1572, .adv_w = 398, .box_w = 24, .box_h = 25, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 1647, .adv_w = 370, .box_w = 22, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1711, .adv_w = 342, .box_w = 20, .box_h = 24, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 1771, .adv_w = 313, .box_w = 19, .box_h = 23, .ofs_x = 5, .ofs_y = 0},
    {.bitmap_index = 1826, .adv_w = 370, .box_w = 22, .box_h = 24, .ofs_x = 4, .ofs_y = -1},
    {.bitmap_index = 1892, .adv_w = 342, .box_w = 20, .box_h = 23, .ofs_x = 6, .ofs_y = 0},
    {.bitmap_index = 1950, .adv_w = 483, .box_w = 29, .box_h = 23, .ofs_x = 6, .ofs_y = 0},
    {.bitmap_index = 2034, .adv_w = 342, .box_w = 24, .box_h = 23, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2103, .adv_w = 342, .box_w = 21, .box_h = 23, .ofs_x = 5, .ofs_y = 0},
    {.bitmap_index = 2164, .adv_w = 313, .box_w = 23, .box_h = 23, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2231, .adv_w = 142, .box_w = 12, .box_h = 30, .ofs_x = 1, .ofs_y = -7},
    {.bitmap_index = 2276, .adv_w = 142, .box_w = 13, .box_h = 30, .ofs_x = -1, .ofs_y = -7},
    {.bitmap_index = 2325, .adv_w = 285, .box_w = 16, .box_h = 18, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 2361, .adv_w = 285, .box_w = 17, .box_h = 24, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 2412, .adv_w = 256, .box_w = 16, .box_h = 18, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 2448, .adv_w = 285, .box_w = 19, .box_h = 24, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 2505, .adv_w = 285, .box_w = 16, .box_h = 18, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 2541, .adv_w = 142, .box_w = 10, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2570, .adv_w = 285, .box_w = 18, .box_h = 24, .ofs_x = 1, .ofs_y = -7},
    {.bitmap_index = 2624, .adv_w = 285, .box_w = 16, .box_h = 23, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 2670, .adv_w = 114, .box_w = 8, .box_h = 23, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 2693, .adv_w = 114, .box_w = 12, .box_h = 30, .ofs_x = -2, .ofs_y = -7},
    {.bitmap_index = 2738, .adv_w = 256, .box_w = 17, .box_h = 23, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 2787, .adv_w = 114, .box_w = 8, .box_h = 23, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 2810, .adv_w = 426, .box_w = 25, .box_h = 17, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 2864, .adv_w = 285, .box_w = 16, .box_h = 17, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 2898, .adv_w = 285, .box_w = 15, .box_h = 18, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 2932, .adv_w = 285, .box_w = 19, .box_h = 24, .ofs_x = 0, .ofs_y = -7},
    {.bitmap_index = 2989, .adv_w = 285, .box_w = 17, .box_h = 24, .ofs_x = 2, .ofs_y = -7},
    {.bitmap_index = 3040, .adv_w = 170, .box_w = 12, .box_h = 17, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3066, .adv_w = 256, .box_w = 15, .box_h = 18, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 3100, .adv_w = 142, .box_w = 9, .box_h = 23, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 3126, .adv_w = 285, .box_w = 16, .box_h = 18, .ofs_x = 3, .ofs_y = -1},
    {.bitmap_index = 3162, .adv_w = 256, .box_w = 15, .box_h = 17, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 3194, .adv_w = 370, .box_w = 22, .box_h = 17, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 3241, .adv_w = 256, .box_w = 18, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3280, .adv_w = 256, .box_w = 19, .box_h = 24, .ofs_x = 0, .ofs_y = -7},
    {.bitmap_index = 3337, .adv_w = 256, .box_w = 17, .box_h = 17, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_0[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 2, 3, 4, 0,
    5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 0, 0, 15, 0, 16, 17,
    0, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 0, 45
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 62, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_0, .list_length = 62, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    },
    {
        .range_start = 97, .range_length = 26, .glyph_id_start = 47,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 2, 3, 4, 4, 5,
    6, 4, 0, 0, 7, 8, 9, 0,
    0, 10, 11, 10, 12, 13, 14, 0,
    15, 15, 8, 15, 8, 0, 0, 16,
    17, 17, 0, 17, 18, 0, 19, 0,
    0, 20, 0, 19, 19, 17, 17, 0,
    21, 22, 23, 0, 24, 24, 20, 24,
    25
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 2, 0, 0,
    0, 2, 0, 0, 3, 0, 0, 0,
    0, 2, 0, 2, 0, 4, 5, 0,
    6, 6, 7, 6, 7, 0, 0, 8,
    0, 9, 9, 9, 10, 8, 0, 0,
    11, 0, 0, 12, 12, 9, 13, 9,
    14, 15, 10, 16, 17, 17, 18, 17,
    12
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    -31, -10, -10, -31, -26, -31, -31, -20,
    0, -26, -15, -20, -20, -20, -26, -20,
    -31, -31, 0, -20, 0, -15, -20, -31,
    -5, -10, -10, -15, 0, -15, -10, -15,
    -10, -13, -15, 0, -15, -15, 0, -10,
    -26, -26, -26, -5, 0, -5, -10, -15,
    -5, -10, -5, -10, -15, -15, -20, -10,
    0, -5, -20, -26, -20, -10, 0, -10,
    -15, -15, -10, -10, -10, -15, -5, -15,
    -15, -20, -10, -26, -10, -26, -15, -15,
    -15, -15, -15, -20, -10, -15, -10, -23,
    -20, -15, -20, -15, -15, -20, 0, -10,
    -15, -15, -15, -10, -10, -26, -15, -20,
    -20, -26, -5, -20, -15, -15, 0, -15,
    0, -26, -26, -15, -10, 0, -10, -20,
    -10, -15, -10, -20, -15, -26, 0, -26,
    0, -26, 0, 0, 0, -15, -20, -15,
    -5, -20, -15, -20, -10, -20, -15, -10,
    0, -20, 0, -20, -20, -31, 0, -10,
    -15, -15, -5, -20, 0, -15, -10, -20,
    -20, 0, -15, 0, 0, -10, -20, -20,
    -26, -10, 0, 0, 0, -10, -5, -10,
    0, -8, 0, 0, -31, -10, -20, -15,
    -15, -20, -31, -26, -20, 0, 0, -20,
    -10, -15, -15, -20, 0, -20, -5, -5,
    0, -10, -10, -26, -10, -5, -5, 0,
    -5, -5, -5, -10, -5, -20, -5, 0,
    -15, -10, 0, -5, -20, -26, -26, 0,
    0, -5, -5, -10, -5, -10, 0, -13,
    -10, -20, -26, -20, -20, -20, 0, -5,
    -20, -20, -20, -10, 0, -20, -15, -20,
    -20, -20, -10, -15, -31, -26, -15, -26,
    0, 0, -15, -26, -26, -10, 0, -31,
    -20, -31, -20, -26, 0, -15, 0, 0,
    0, 0, 0, 0, 0, -5, 0, -5,
    0, -5, 0, -5, -5, -5, -10, -5,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -5, 0, -5, 0, -5, -5, -3,
    -5, -10, 0, 0, 0, 0, 0, 0,
    0, -10, -10, 0, 0, -5, 0, -5,
    -10, -3, 0, -10, 0, 0, 0, 0,
    0, 0, 0, 0, -5, 0, -10, -13,
    -5, -10, -10, -10, -10, -10, 0, 0,
    0, 0, 0, 0, 0, -5, -10, 0,
    0, -10, -5, -10, -5, -10, -10, 0,
    0, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 0, -5, 0, 0, -5, -3,
    0, -5, 0, 0, 0, 0, 0, 0,
    0, -5, 0, -10, -10, -10, -5, -10,
    -5, -8, -10, -20, 0, 0, 0, 0,
    0, 0, 0, -5, -5, -8, 0, -5,
    0, -5, -5, 0, 0, -10, 0, 0,
    0, 0, 0, 0, 0, -10, -10, -5,
    -10, -20, -15, -13, -10, -15, 0, -10,
    0, 0, 0, 0, 0, 0, 0, -10,
    -10, -5, -5, -15, -10, -15, -10, -18,
    -10, -10
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 25,
    .right_class_cnt     = 18,
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 1,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t FreeSansOblique32 = {
#else
lv_font_t FreeSansOblique32 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 30,          /*The maximum line height required by the font*/
    .base_line = 7,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0)
    .underline_position = -6,
    .underline_thickness = 2,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if FREESANSOBLIQUE32*/


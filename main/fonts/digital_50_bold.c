/*******************************************************************************
 * Size: 50 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#include "lvgl.h"

#ifndef DIGITAL_50_BOLD
#define DIGITAL_50_BOLD 1
#endif

#if DIGITAL_50_BOLD

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0030 "0" */
    0x7f, 0xff, 0xeb, 0xff, 0xfd, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf7, 0xfe, 0xff, 0x80, 0x1f, 0xf8,
    0x1, 0xff, 0x80, 0x1f, 0xf8, 0x1, 0xff, 0x80,
    0x1f, 0xf8, 0x1, 0xff, 0x80, 0x1f, 0xf8, 0x1,
    0xff, 0x0, 0xf, 0x60, 0x0, 0x60, 0x0, 0x0,
    0x0, 0x0, 0x6, 0x0, 0x6, 0xf0, 0x0, 0xff,
    0x80, 0x1f, 0xf8, 0x1, 0xff, 0x80, 0x1f, 0xf8,
    0x1, 0xff, 0x80, 0x1f, 0xf8, 0x1, 0xff, 0x80,
    0x1f, 0xf8, 0x1, 0xff, 0x7f, 0xef, 0xef, 0xff,
    0x7d, 0xff, 0xfb, 0xbf, 0xff, 0xd7, 0xff, 0xfe,

    /* U+0031 "1" */
    0x8, 0xce, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf7,
    0x98, 0x0, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xde, 0x71, 0x84,

    /* U+0032 "2" */
    0x7f, 0xff, 0xe3, 0xff, 0xfd, 0x1f, 0xff, 0xb0,
    0xff, 0xf7, 0x7, 0xfe, 0xf0, 0x0, 0x1f, 0x0,
    0x1, 0xf0, 0x0, 0x1f, 0x0, 0x1, 0xf0, 0x0,
    0x1f, 0x0, 0x1, 0xf0, 0x0, 0x1f, 0x0, 0x1,
    0xf0, 0x0, 0xf, 0x7, 0xfe, 0x60, 0xff, 0xf0,
    0x1f, 0xff, 0x80, 0xff, 0xf0, 0x67, 0xfe, 0xf,
    0x0, 0x0, 0xf8, 0x0, 0xf, 0x80, 0x0, 0xf8,
    0x0, 0xf, 0x80, 0x0, 0xf8, 0x0, 0xf, 0x80,
    0x0, 0xf8, 0x0, 0xf, 0x7f, 0xe0, 0xef, 0xff,
    0xd, 0xff, 0xf8, 0xbf, 0xff, 0xc7, 0xff, 0xfe,

    /* U+0033 "3" */
    0x7f, 0xff, 0xe3, 0xff, 0xfd, 0x1f, 0xff, 0xb0,
    0xff, 0xf7, 0x7, 0xfe, 0xf0, 0x0, 0x1f, 0x0,
    0x1, 0xf0, 0x0, 0x1f, 0x0, 0x1, 0xf0, 0x0,
    0x1f, 0x0, 0x1, 0xf0, 0x0, 0x1f, 0x0, 0x1,
    0xf0, 0x0, 0xf, 0xf, 0xfe, 0x61, 0xff, 0xf0,
    0x3f, 0xff, 0x81, 0xff, 0xf0, 0xf, 0xfe, 0x60,
    0x0, 0xf, 0x0, 0x1, 0xf0, 0x0, 0x1f, 0x0,
    0x1, 0xf0, 0x0, 0x1f, 0x0, 0x1, 0xf0, 0x0,
    0x1f, 0x0, 0x1, 0xf0, 0x7f, 0xef, 0xf, 0xff,
    0x71, 0xff, 0xfb, 0x3f, 0xff, 0xd7, 0xff, 0xfe,

    /* U+0034 "4" */
    0x0, 0x0, 0x8, 0x0, 0x1, 0xc0, 0x0, 0x3e,
    0x0, 0x7, 0xf0, 0x0, 0xff, 0x80, 0x1f, 0xf8,
    0x1, 0xff, 0x80, 0x1f, 0xf8, 0x1, 0xff, 0x80,
    0x1f, 0xf8, 0x1, 0xff, 0x80, 0x1f, 0xf0, 0x0,
    0xf6, 0x7f, 0xe6, 0xf, 0xff, 0x1, 0xff, 0xf8,
    0xf, 0xff, 0x0, 0x7f, 0xe6, 0x0, 0x0, 0xf0,
    0x0, 0x1f, 0x0, 0x1, 0xf0, 0x0, 0x1f, 0x0,
    0x1, 0xf0, 0x0, 0x1f, 0x0, 0x1, 0xf0, 0x0,
    0x1f, 0x0, 0x0, 0xf0, 0x0, 0x7, 0x0, 0x0,
    0x30, 0x0, 0x1, 0x0, 0x0, 0x0,

    /* U+0035 "5" */
    0x7f, 0xff, 0xeb, 0xff, 0xfc, 0xdf, 0xff, 0x8e,
    0xff, 0xf0, 0xf7, 0xfe, 0xf, 0x80, 0x0, 0xf8,
    0x0, 0xf, 0x80, 0x0, 0xf8, 0x0, 0xf, 0x80,
    0x0, 0xf8, 0x0, 0xf, 0x80, 0x0, 0xf8, 0x0,
    0xf, 0x0, 0x0, 0x67, 0xfe, 0x0, 0xff, 0xf0,
    0x1f, 0xff, 0x80, 0xff, 0xf0, 0x7, 0xfe, 0x60,
    0x0, 0xf, 0x0, 0x1, 0xf0, 0x0, 0x1f, 0x0,
    0x1, 0xf0, 0x0, 0x1f, 0x0, 0x1, 0xf0, 0x0,
    0x1f, 0x0, 0x1, 0xf0, 0x7f, 0xef, 0xf, 0xff,
    0x71, 0xff, 0xfb, 0x3f, 0xff, 0xd7, 0xff, 0xfe,

    /* U+0036 "6" */
    0x7f, 0xff, 0xeb, 0xff, 0xfc, 0xdf, 0xff, 0x8e,
    0xff, 0xf0, 0xf7, 0xfe, 0xf, 0x80, 0x0, 0xf8,
    0x0, 0xf, 0x80, 0x0, 0xf8, 0x0, 0xf, 0x80,
    0x0, 0xf8, 0x0, 0xf, 0x80, 0x0, 0xf8, 0x0,
    0xf, 0x0, 0x0, 0x67, 0xfe, 0x0, 0xff, 0xf0,
    0x1f, 0xff, 0x80, 0xff, 0xf0, 0x67, 0xfe, 0x6f,
    0x0, 0xf, 0xf8, 0x1, 0xff, 0x80, 0x1f, 0xf8,
    0x1, 0xff, 0x80, 0x1f, 0xf8, 0x1, 0xff, 0x80,
    0x1f, 0xf8, 0x1, 0xff, 0x7f, 0xef, 0xef, 0xff,
    0x7d, 0xff, 0xfb, 0xbf, 0xff, 0xd7, 0xff, 0xfe,

    /* U+0037 "7" */
    0x7f, 0xff, 0xe3, 0xff, 0xfd, 0x1f, 0xff, 0xb0,
    0xff, 0xf7, 0x7, 0xfe, 0xf0, 0x0, 0x1f, 0x0,
    0x1, 0xf0, 0x0, 0x1f, 0x0, 0x1, 0xf0, 0x0,
    0x1f, 0x0, 0x1, 0xf0, 0x0, 0x1f, 0x0, 0x1,
    0xf0, 0x0, 0xf, 0x0, 0x0, 0x60, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0x70,
    0x0, 0xf, 0x0, 0x1, 0xf0, 0x0, 0x1f, 0x0,
    0x1, 0xf0, 0x0, 0x1f, 0x0, 0x1, 0xf0, 0x0,
    0x1f, 0x0, 0x1, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0x70, 0x0, 0x3, 0x0, 0x0, 0x10,

    /* U+0038 "8" */
    0x7f, 0xff, 0xeb, 0xff, 0xfd, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf7, 0xfe, 0xff, 0x80, 0x1f, 0xf8,
    0x1, 0xff, 0x80, 0x1f, 0xf8, 0x1, 0xff, 0x80,
    0x1f, 0xf8, 0x1, 0xff, 0x80, 0x1f, 0xf8, 0x1,
    0xff, 0x0, 0xf, 0x67, 0xfe, 0x60, 0xff, 0xf0,
    0x1f, 0xff, 0x80, 0xff, 0xf0, 0x67, 0xfe, 0x6f,
    0x0, 0xf, 0xf8, 0x1, 0xff, 0x80, 0x1f, 0xf8,
    0x1, 0xff, 0x80, 0x1f, 0xf8, 0x1, 0xff, 0x80,
    0x1f, 0xf8, 0x1, 0xff, 0x7f, 0xef, 0xef, 0xff,
    0x7d, 0xff, 0xfb, 0xbf, 0xff, 0xd7, 0xff, 0xfe,

    /* U+0039 "9" */
    0x7f, 0xff, 0xeb, 0xff, 0xfd, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf7, 0xfe, 0xff, 0x80, 0x1f, 0xf8,
    0x1, 0xff, 0x80, 0x1f, 0xf8, 0x1, 0xff, 0x80,
    0x1f, 0xf8, 0x1, 0xff, 0x80, 0x1f, 0xf8, 0x1,
    0xff, 0x0, 0xf, 0x67, 0xfe, 0x60, 0xff, 0xf0,
    0x1f, 0xff, 0x80, 0xff, 0xf0, 0x7, 0xfe, 0x60,
    0x0, 0xf, 0x0, 0x1, 0xf0, 0x0, 0x1f, 0x0,
    0x1, 0xf0, 0x0, 0x1f, 0x0, 0x1, 0xf0, 0x0,
    0x1f, 0x0, 0x1, 0xf0, 0x7f, 0xef, 0xf, 0xff,
    0x71, 0xff, 0xfb, 0x3f, 0xff, 0xd7, 0xff, 0xfe,

    /* U+003A ":" */
    0xff, 0xff, 0xff, 0x80, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0xff, 0xff,
    0xfe
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 407, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 80, .adv_w = 228, .box_w = 5, .box_h = 30, .ofs_x = 5, .ofs_y = 1},
    {.bitmap_index = 99, .adv_w = 407, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 179, .adv_w = 407, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 407, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 337, .adv_w = 407, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 417, .adv_w = 407, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 497, .adv_w = 407, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 575, .adv_w = 407, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 655, .adv_w = 407, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 735, .adv_w = 177, .box_w = 5, .box_h = 27, .ofs_x = 3, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 48, .range_length = 11, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
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
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
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
const lv_font_t digital_50_bold = {
#else
lv_font_t digital_50_bold = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 32,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -5,
    .underline_thickness = 3,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if DIGITAL_50_BOLD*/

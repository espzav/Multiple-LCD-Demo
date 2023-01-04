/*******************************************************************************
 * Size: 30 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#include "lvgl.h"

#ifndef DIGITAL_30
#define DIGITAL_30 1
#endif

#if DIGITAL_30

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+002E "." */
    0xff, 0x80,

    /* U+0030 "0" */
    0x7f, 0xeb, 0xfd, 0xdf, 0xbe, 0x7, 0xe0, 0x7e,
    0x7, 0xe0, 0x7e, 0x7, 0x40, 0x20, 0x0, 0x40,
    0x2e, 0x7, 0xe0, 0x7e, 0x7, 0xe0, 0x7e, 0x7,
    0xdf, 0xbb, 0xfd, 0x7f, 0xe0,

    /* U+0031 "1" */
    0x2f, 0xff, 0xfa, 0xb, 0xff, 0xfb, 0x20,

    /* U+0032 "2" */
    0x7f, 0xe3, 0xfd, 0x1f, 0xb0, 0x7, 0x0, 0x70,
    0x7, 0x0, 0x70, 0x7, 0x1f, 0xa3, 0xfc, 0x5f,
    0x8e, 0x0, 0xe0, 0xe, 0x0, 0xe0, 0xe, 0x0,
    0xdf, 0x8b, 0xfc, 0x7f, 0xe0,

    /* U+0033 "3" */
    0x7f, 0xe3, 0xfd, 0x1f, 0xb0, 0x7, 0x0, 0x70,
    0x7, 0x0, 0x70, 0x7, 0x3f, 0xa7, 0xfc, 0x3f,
    0xa0, 0x7, 0x0, 0x70, 0x7, 0x0, 0x70, 0x7,
    0x1f, 0xb3, 0xfd, 0x7f, 0xe0,

    /* U+0034 "4" */
    0x0, 0x8, 0x1, 0xc0, 0x3e, 0x7, 0xe0, 0x7e,
    0x7, 0xe0, 0x7e, 0x7, 0x5f, 0xa3, 0xfc, 0x1f,
    0xa0, 0x7, 0x0, 0x70, 0x7, 0x0, 0x70, 0x7,
    0x0, 0x30, 0x1, 0x0, 0x0,

    /* U+0035 "5" */
    0x7f, 0xeb, 0xfc, 0xdf, 0x8e, 0x0, 0xe0, 0xe,
    0x0, 0xe0, 0xe, 0x0, 0x5f, 0x83, 0xfc, 0x1f,
    0xa0, 0x7, 0x0, 0x70, 0x7, 0x0, 0x70, 0x7,
    0x1f, 0xb3, 0xfd, 0x7f, 0xe0,

    /* U+0036 "6" */
    0x7f, 0xeb, 0xfc, 0xdf, 0x8e, 0x0, 0xe0, 0xe,
    0x0, 0xe0, 0xe, 0x0, 0x5f, 0x83, 0xfc, 0x5f,
    0xae, 0x7, 0xe0, 0x7e, 0x7, 0xe0, 0x7e, 0x7,
    0xdf, 0xbb, 0xfd, 0x7f, 0xe0,

    /* U+0037 "7" */
    0x7f, 0xe3, 0xfd, 0x1f, 0xb0, 0x7, 0x0, 0x70,
    0x7, 0x0, 0x70, 0x7, 0x0, 0x30, 0x0, 0x0,
    0x0, 0x7, 0x0, 0x70, 0x7, 0x0, 0x70, 0x7,
    0x0, 0x30, 0x1, 0x0, 0x0,

    /* U+0038 "8" */
    0x7f, 0xeb, 0xfd, 0xdf, 0xbe, 0x7, 0xe0, 0x7e,
    0x7, 0xe0, 0x7e, 0x7, 0x5f, 0xa3, 0xfc, 0x5f,
    0xae, 0x7, 0xe0, 0x7e, 0x7, 0xe0, 0x7e, 0x7,
    0xdf, 0xbb, 0xfd, 0x7f, 0xe0,

    /* U+0039 "9" */
    0x7f, 0xeb, 0xfd, 0xdf, 0xbe, 0x7, 0xe0, 0x7e,
    0x7, 0xe0, 0x7e, 0x7, 0x5f, 0xa3, 0xfc, 0x1f,
    0xa0, 0x7, 0x0, 0x70, 0x7, 0x0, 0x70, 0x7,
    0x1f, 0xb3, 0xfd, 0x7f, 0xe0,

    /* U+003A ":" */
    0xff, 0x80, 0x0, 0x0, 0x1, 0xff
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 86, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 244, .box_w = 12, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 31, .adv_w = 137, .box_w = 3, .box_h = 17, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 38, .adv_w = 244, .box_w = 12, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 67, .adv_w = 244, .box_w = 12, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 96, .adv_w = 244, .box_w = 12, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 244, .box_w = 12, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 244, .box_w = 12, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 183, .adv_w = 244, .box_w = 12, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 212, .adv_w = 244, .box_w = 12, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 241, .adv_w = 244, .box_w = 12, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 270, .adv_w = 106, .box_w = 3, .box_h = 16, .ofs_x = 2, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_0[] = {
    0, 0, 1, 2, 3, 4, 5, 6,
    7, 8, 9, 10, 11
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 46, .range_length = 13, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_0, .list_length = 13, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
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
const lv_font_t digital_30 = {
#else
lv_font_t digital_30 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 19,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -3,
    .underline_thickness = 2,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if DIGITAL_30*/


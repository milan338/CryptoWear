#ifndef VIEWDATA_H
#define VIEWDATA_H

#include <cairo.h>
#include "main.h"

// Colors
#define VIEW_DATA_BACKGROUND_COLOR RGBA_NORM(0, 0, 0, 255)
#define VIEW_DATA_TEXT_COLOR RGBA_NORM(255, 255, 255, 255)
// Line styles
#define VIEW_DATA_PRIMARY_LINE_WIDTH 50.0
// Font styles
#define VIEW_DATA_FONT_FACE "Sans"
#define VIEW_DATA_FONT_SLANT CAIRO_FONT_SLANT_NORMAL
#define VIEW_DATA_FONT_WEIGHT CAIRO_FONT_WEIGHT_NORMAL
#define VIEW_DATA_PRIMARY_FONT_SIZE 64.0
#define VIEW_DATA_SECONDARY_FONT_SIZE 32.0
#define VIEW_DATA_TERTIARY_FONT_SIZE 24.0
#define VIEW_DATA_SYMBOL_FONT_SIZE 20.0
// Formatting
#define VIEW_DATA_MAX_D_BAL_WITH_DECIMAL 100000
#define VIEW_DATA_MAX_D_BAL_NON_SCIENTIFIC 10000000
#define VIEW_DATA_MAX_C_BAL_WITH_DECIMAL 1000000
#define VIEW_DATA_MAX_C_BAL_NON_SCIENTIFIC 1000000000000

typedef struct MoreOptionData
{
    char *text;
    char *img;
    void *cb;
} MoreOptionData;

void view_data_init(appdata_s *ad);

void view_data_text_setup(cairo_t *cairo);

void view_data_populate_more_option(int i);

Elm_Object_Item *view_data_push(int i);

void more_option_opened_cb(void *data, Evas_Object *obj, void *event_info);

#endif // VIEWDATA_H

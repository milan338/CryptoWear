#ifndef DRAW_H
#define DRAW_H

#include <cairo.h>
#include "main.h"

#define SMALL_CURRENCY_LABEL_FONT_SIZE

void cairo_image_new(appdata_s *ad);
void cairo_image_update(appdata_s *ad);

void cairo_image_draw_with_symbol(cairo_text_extents_t *extents, char *line, double main_size, double symbol_size, double x, double y);
void cairo_image_draw_balance(cairo_text_extents_t *extents, int decimal_max, int n_scientific_max, double main_size, double symbol_size, double x, double y, double bal);
void cairo_image_draw_double(cairo_text_extents_t *extents, int decimal_max, int n_scientific_max, double size, double x, double y, double bal);

#endif // DRAW_H

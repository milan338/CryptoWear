#include <bundle.h>
#include <cairo.h>
#include "main.h"
#include "draw.h"

// Begin new cairo surface image
void cairo_image_new(appdata_s *ad)
{
    // Init cairo
    ad->cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ad->width, ad->height);
    ad->cairo = cairo_create(ad->cairo_surface);
    // Init image
    ad->image = evas_object_image_add(evas_object_evas_get(ad->naviframe));
    evas_object_show(ad->image);
    evas_object_image_size_set(ad->image, ad->width, ad->height);
    evas_object_image_fill_set(ad->image, 0, 0, ad->width, ad->height);
}

// Update naviframe image with cairo surface
void cairo_image_update(appdata_s *ad)
{
    // Finish drawing
    cairo_surface_flush(ad->cairo_surface);
    // Update image data
    unsigned char *image_data = cairo_image_surface_get_data(cairo_get_target(ad->cairo));
    evas_object_image_data_set(ad->image, image_data);
    evas_object_image_data_update_add(ad->image, 0, 0, ad->width, ad->height);
}

// Draw string with small currency symbol in top left
void cairo_image_draw_with_symbol(cairo_text_extents_t *extents, char *line, double main_size, double symbol_size, double x, double y)
{
    appdata_s *ad = get_appdata(NULL);
    // Get currency symbol
    char *symbol = fiat_symbol();
    // Get dimensions of symbol text
    cairo_set_font_size(ad->cairo, symbol_size);
    cairo_text_extents(ad->cairo, symbol, extents);
    double symbol_w = extents->width;
    double symbol_x_b = extents->x_bearing;
    double symbol_h = extents->height;
    // Draw main line
    cairo_set_font_size(ad->cairo, main_size);
    cairo_move_to(ad->cairo, x, y);
    cairo_text_extents(ad->cairo, line, extents);
    cairo_move_to(ad->cairo,
                  x - (((extents->width + symbol_w) / 2.0) + extents->x_bearing + symbol_x_b) + symbol_w,
                  y - ((extents->height / 2.0) + extents->y_bearing));
    cairo_show_text(ad->cairo, line);
    // Draw symbol
    cairo_move_to(ad->cairo,
                  x - (((extents->width + symbol_w) / 2.0) + extents->x_bearing + symbol_x_b),
                  y - ((extents->height / 2.0) + extents->y_bearing + (extents->height - symbol_h)));
    cairo_set_font_size(ad->cairo, symbol_size);
    cairo_show_text(ad->cairo, symbol);
}

// Draw balance with small currency symbol in top left
void cairo_image_draw_balance(cairo_text_extents_t *extents, int decimal_max, int n_scientific_max, double main_size, double symbol_size, double x, double y, double bal)
{
    char *line = d_balance_str_fmt("",
                                   bal,
                                   decimal_max,
                                   n_scientific_max);
    cairo_image_draw_with_symbol(extents, line, main_size, symbol_size, x, y);
    free(line);
}

// Draw double
void cairo_image_draw_double(cairo_text_extents_t *extents, int decimal_max, int n_scientific_max, double size, double x, double y, double bal)
{
    appdata_s *ad = get_appdata(NULL);
    char *line = d_balance_str_fmt("",
                                   bal,
                                   decimal_max,
                                   n_scientific_max);
    // Draw text
    cairo_set_font_size(ad->cairo, size);
    cairo_move_to(ad->cairo, x, y);
    cairo_text_extents(ad->cairo, line, extents);
    cairo_move_to(ad->cairo,
                  x - ((extents->width / 2.0) + extents->x_bearing),
                  y - ((extents->height / 2.0) + extents->y_bearing));
    cairo_show_text(ad->cairo, line);
    // Cleanup
    free(line);
}

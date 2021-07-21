#include <bundle.h>
#include <cairo.h>
#include "main.h"
#include "../../draw.h"
#include "view_data.h"
#include "view_data_volume.h"

void view_data_volume(int i)
{
    appdata_s *ad = get_appdata(NULL);
    char *symbol = ad->coin_data[i].symbol;
    // Init view
    view_data_init(ad);
    // Screen centre coordinates
    double x = ad->width / 2.0;
    double y = ad->height / 2.0;
    // Setup text drawing
    cairo_text_extents_t extents = {};
    view_data_text_setup(ad->cairo);
    // Get coin data
    CoinPriceData *price_data;
    size_t size;
    eina_lock_take(&ad->coin_price_data_mutex);
    bundle_get_byte(ad->coin_price_data, symbol, (void **)&price_data, &size);
    double high = price_data->high;
    double low = price_data->low;
    double open = price_data->open;
    double volume = price_data->volume;
    eina_lock_release(&ad->coin_price_data_mutex);
    char *line;
    // Draw open price
    cairo_image_draw_double(&extents,
                            VIEW_DATA_MAX_D_BAL_WITH_DECIMAL,
                            VIEW_DATA_MAX_D_BAL_NON_SCIENTIFIC,
                            VIEW_DATA_SECONDARY_FONT_SIZE,
                            x / 2.0, y - (ad->height * 0.1), open);
    // Draw volume
    line = c_balance_str_fmt_suffix("", volume);
    cairo_set_font_size(ad->cairo, VIEW_DATA_SECONDARY_FONT_SIZE);
    cairo_text_extents(ad->cairo, line, &extents);
    cairo_move_to(ad->cairo,
                  x + (x / 2.0) - ((extents.width / 2.0) + extents.x_bearing),
                  y - (ad->height * 0.1) - ((extents.height / 2.0) + extents.y_bearing));
    cairo_show_text(ad->cairo, line);
    free(line);
    // Draw low price
    cairo_image_draw_double(&extents,
                            VIEW_DATA_MAX_D_BAL_WITH_DECIMAL,
                            VIEW_DATA_MAX_D_BAL_NON_SCIENTIFIC,
                            VIEW_DATA_SECONDARY_FONT_SIZE,
                            x / 2.0, y + (ad->height * 0.16), low);
    // Draw high price
    cairo_image_draw_double(&extents,
                            VIEW_DATA_MAX_D_BAL_WITH_DECIMAL,
                            VIEW_DATA_MAX_D_BAL_NON_SCIENTIFIC,
                            VIEW_DATA_SECONDARY_FONT_SIZE,
                            x + (x / 2.0), y + (ad->height * 0.16), high);
    // Draw labels
    cairo_set_source_rgba(ad->cairo, COLOR_SECONDARY_RGBA);
    line = "OPEN";
    cairo_text_extents(ad->cairo, line, &extents);
    cairo_move_to(ad->cairo,
                  (x / 2.0) - ((extents.width / 2.0) + extents.x_bearing),
                  y - (ad->height * 0.1) - ((extents.height / 2.0) + extents.y_bearing) - (ad->height * 0.1));
    cairo_show_text(ad->cairo, line);
    line = "VOLUME";
    cairo_text_extents(ad->cairo, line, &extents);
    cairo_move_to(ad->cairo,
                  x + (x / 2.0) - ((extents.width / 2.0) + extents.x_bearing),
                  y - (ad->height * 0.1) - ((extents.height / 2.0) + extents.y_bearing) - (ad->height * 0.1));
    cairo_show_text(ad->cairo, line);
    line = "LOW";
    cairo_text_extents(ad->cairo, line, &extents);
    cairo_move_to(ad->cairo,
                  (x / 2.0) - ((extents.width / 2.0) + extents.x_bearing),
                  y + (ad->height * 0.16) - ((extents.height / 2.0) + extents.y_bearing) - (ad->height * 0.1));
    cairo_show_text(ad->cairo, line);
    line = "HIGH";
    cairo_text_extents(ad->cairo, line, &extents);
    cairo_move_to(ad->cairo,
                  x + (x / 2.0) - ((extents.width / 2.0) + extents.x_bearing),
                  y + (ad->height * 0.16) - ((extents.height / 2.0) + extents.y_bearing) - (ad->height * 0.1));
    cairo_show_text(ad->cairo, line);
    // Draw pair
    cairo_set_source_rgba(ad->cairo, RGBA_NORM(255, 255, 255, 255));
    line = new_string_printf("%s%s", symbol, ad->currency);
    cairo_text_extents(ad->cairo, line, &extents);
    cairo_move_to(ad->cairo,
                  x - ((extents.width / 2.0) + extents.x_bearing),
                  y - ((extents.height / 2.0) + extents.y_bearing + (ad->height * 0.37)));
    cairo_show_text(ad->cairo, line);
    free(line);
    // Finish drawing and update image data
    cairo_image_update(ad);
    // Push view to naviframe
    view_data_push(i);
}

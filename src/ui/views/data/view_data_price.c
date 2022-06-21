#include <bundle.h>
#include <cairo.h>
#include "main.h"
#include "../../draw.h"
#include "view_data.h"
#include "view_data_price.h"

void view_data_price(int i)
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
    char *line;
    // Get coin data
    CoinPriceData *price_data;
    size_t size;
    lock_take(&ad->coin_price_data_mutex);
    bundle_get_byte(ad->coin_price_data, symbol, (void **)&price_data, &size);
    double price = price_data->price;
    double low = price_data->low;
    double high = price_data->high;
    double percent_change = price_data->percent_change;
    double absolute_change = price_data->absolute_change;
    lock_release(&ad->coin_price_data_mutex);
    // Draw coin price
    cairo_image_draw_balance(&extents,
                             VIEW_DATA_MAX_D_BAL_WITH_DECIMAL,
                             VIEW_DATA_MAX_D_BAL_NON_SCIENTIFIC,
                             VIEW_DATA_PRIMARY_FONT_SIZE,
                             VIEW_DATA_SYMBOL_FONT_SIZE,
                             x, y - (ad->height * 0.15), price);
    // Draw coin movement
    cairo_set_font_size(ad->cairo, VIEW_DATA_SECONDARY_FONT_SIZE);
    char *sign = "";
    if (percent_change < 0.0)
        cairo_set_source_rgba(ad->cairo, COLOR_DECREASE_RGBA);
    else if (percent_change > 0.0)
    {
        cairo_set_source_rgba(ad->cairo, COLOR_INCREASE_RGBA);
        sign = "+";
    }
    else
        cairo_set_source_rgba(ad->cairo, COLOR_SAME_RGBA);
    char *_abs_change = d_balance_str_fmt("", absolute_change,
                                          VIEW_DATA_MAX_D_BAL_WITH_DECIMAL,
                                          VIEW_DATA_MAX_D_BAL_NON_SCIENTIFIC);
    line = new_string_printf("%s%s (%s%.2f%%)", sign, _abs_change, sign, percent_change);
    free(_abs_change);
    cairo_text_extents(ad->cairo, line, &extents);
    cairo_move_to(ad->cairo,
                  x - ((extents.width / 2.0) + extents.x_bearing),
                  y - ((extents.height / 2.0) + extents.y_bearing + (ad->height * 0.01)));
    cairo_show_text(ad->cairo, line);
    free(line);
    // Draw coin low / high
    cairo_set_source_rgba(ad->cairo, RGBA_NORM(255, 255, 255, 255));
    line = d_balance_str_fmt("", low,
                             VIEW_DATA_MAX_D_BAL_WITH_DECIMAL,
                             VIEW_DATA_MAX_D_BAL_NON_SCIENTIFIC);
    cairo_text_extents(ad->cairo, line, &extents);
    cairo_move_to(ad->cairo,
                  x - ((extents.width / 2.0) + extents.x_bearing + (ad->width * 0.2)),
                  y - ((extents.height / 2.0) + extents.y_bearing) + (ad->height * 0.23));
    cairo_show_text(ad->cairo, line);
    free(line);
    line = d_balance_str_fmt("", high,
                             VIEW_DATA_MAX_D_BAL_WITH_DECIMAL,
                             VIEW_DATA_MAX_D_BAL_NON_SCIENTIFIC);
    cairo_text_extents(ad->cairo, line, &extents);
    cairo_move_to(ad->cairo,
                  x - ((extents.width / 2.0) + extents.x_bearing) + (ad->width * 0.2),
                  y - ((extents.height / 2.0) + extents.y_bearing) + (ad->height * 0.23));
    cairo_show_text(ad->cairo, line);
    free(line);
    // Draw labels
    cairo_set_source_rgba(ad->cairo, COLOR_SECONDARY_RGBA);
    line = "LOW";
    cairo_text_extents(ad->cairo, line, &extents);
    cairo_move_to(ad->cairo,
                  x - ((extents.width / 2.0) + extents.x_bearing + (ad->width * 0.2)),
                  y - ((extents.height / 2.0) + extents.y_bearing) + (ad->height * 0.13));
    cairo_show_text(ad->cairo, line);
    line = "HIGH";
    cairo_text_extents(ad->cairo, line, &extents);
    cairo_move_to(ad->cairo,
                  x - ((extents.width / 2.0) + extents.x_bearing) + (ad->width * 0.2),
                  y - ((extents.height / 2.0) + extents.y_bearing) + (ad->height * 0.13));
    cairo_show_text(ad->cairo, line);
    // Draw divider
    cairo_set_line_width(ad->cairo, 1.0);
    cairo_move_to(ad->cairo, x, y + (ad->height * 0.08));
    cairo_line_to(ad->cairo, x, y + (ad->height * 0.29));
    cairo_stroke(ad->cairo);
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

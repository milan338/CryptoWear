#include <bundle.h>
#include <cairo.h>
#include "main.h"
#include "../../draw.h"
#include "view_data.h"
#include "view_data_main.h"

void view_data_main(int i)
{
    appdata_s *ad = get_appdata(NULL);
    // Init view
    view_data_init(ad);
    // Screen centre coordinates
    double x = ad->width / 2.0;
    double y = ad->height / 2.0;
    // Setup text drawing
    cairo_text_extents_t extents = {};
    view_data_text_setup(ad->cairo);
    char *line;
    // Draw fiat balance
    cairo_image_draw_balance(&extents,
                             VIEW_DATA_MAX_D_BAL_WITH_DECIMAL,
                             VIEW_DATA_MAX_D_BAL_NON_SCIENTIFIC,
                             VIEW_DATA_PRIMARY_FONT_SIZE,
                             VIEW_DATA_SYMBOL_FONT_SIZE,
                             x, y, ad->coin_data[i].d_balance);
    // Draw crypto balance
    cairo_set_font_size(ad->cairo, VIEW_DATA_SECONDARY_FONT_SIZE);
    line = c_balance_str_fmt(ad->coin_data[i].symbol,
                             ad->coin_data[i].c_balance,
                             VIEW_DATA_MAX_C_BAL_WITH_DECIMAL,
                             VIEW_DATA_MAX_C_BAL_NON_SCIENTIFIC);
    cairo_text_extents(ad->cairo, line, &extents);
    cairo_move_to(ad->cairo,
                  x - ((extents.width / 2.0) + extents.x_bearing),
                  y - ((extents.height / 2.0) + extents.y_bearing) + (ad->height * 0.16));
    cairo_show_text(ad->cairo, line);
    free(line);
    // Draw crypto text
    lock_take(&ad->coin_list_mutex);
    // Line is returned from bundle, don't free
    bundle_get_str(ad->crypto_names, ad->coin_data[i].symbol, &line);
    lock_release(&ad->coin_list_mutex);
    cairo_text_extents(ad->cairo, line, &extents);
    cairo_move_to(ad->cairo,
                  x - ((extents.width / 2.0) + extents.x_bearing),
                  y - ((extents.height / 2.0) + extents.y_bearing) - (ad->height * 0.16));
    cairo_show_text(ad->cairo, line);
    // Draw exchange name
    cairo_text_extents(ad->cairo, ad->current_exchange, &extents);
    cairo_move_to(ad->cairo,
                  x - ((extents.width / 2.0) + extents.x_bearing),
                  y - ((extents.height / 2.0) + extents.y_bearing + (ad->height * 0.37)));
    cairo_show_text(ad->cairo, ad->current_exchange);
    // Finish drawing and update image data
    cairo_image_update(ad);
    // Push view to naviframe
    view_data_push(i);
}

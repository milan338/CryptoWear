#include <cairo.h>
#include <math.h>
#include "main.h"
#include "../draw.h"
#include "pie_chart_view.h"

// Cleanup on naviframe pop
static Eina_Bool pie_chart_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = data;
    // Free cairo resources
    cairo_destroy(ad->cairo);
    cairo_surface_destroy(ad->cairo_surface);
    return EINA_TRUE;
}

// Copy contents of one pie chart data array to another
static bool pie_chart_data_copy(CoinData *in_data, CoinData *out_data, size_t in_size, size_t out_size)
{
    // Input needs to be larger than output
    if (out_size > in_size)
        return false;
    for (size_t i = 0; i < out_size; i++)
        out_data[i] = in_data[i];
    return true;
}

// Find data too small to have a significant representation on the pie chart
// Data must be sorted greatest - smallest in terms of dollar balance
static int pie_chart_insignificant_data(CoinData *in_data, size_t in_size, size_t *out_size)
{
    // Total balance
    double total_balance = 0.0;
    for (size_t i = 0; i < in_size; i++)
        total_balance += in_data[i].d_balance;
    // Move insignificant balances to 'other'
    for (size_t i = 0; i < PIE_CHART_MAX_SECTIONS; i++)
    {
        // Balance insignificant if angle on graph less than that of the seperator
        if (in_data[i].d_balance / total_balance * 2 * M_PI < PIE_CHART_SEPERATOR_LINE_THETA)
        {
            *out_size = i;
            return i + 1;
        }
    }
    // No insignificant data
    return -1;
}

// Truncate pie chart from one array to another
// Move excess data into 'other' section
static int pie_chart_data_truncate(CoinData *in_data, CoinData *out_data, size_t in_size)
{
    // Number of sections to draw
    size_t out_size = in_size <= PIE_CHART_MAX_SECTIONS ? in_size : PIE_CHART_MAX_SECTIONS;
    // Determine whether to move insignificant balances into 'other'
    int ret = pie_chart_insignificant_data(in_data, in_size, &out_size);
    // Copy original sections
    pie_chart_data_copy(in_data, out_data, in_size, out_size);
    // Add 'other' entry with data from all omitted entries
    if (in_size > PIE_CHART_MAX_SECTIONS || ret > 0)
    {
        // Total 'other' balances
        double c_balance_total = 0.0;
        double d_balance_total = 0.0;
        for (size_t i = out_size; i < in_size; i++)
        {
            c_balance_total += in_data[i].c_balance;
            d_balance_total += in_data[i].d_balance;
        }
        // Add 'other' entry to data
        CoinData other_data = {"...", c_balance_total, d_balance_total};
        out_data[out_size] = other_data;
    }
    // Insignificant data has not been truncated
    if (ret <= 0)
    {
        // Number of sections to draw
        ret = out_size;
        // Include 'other' section when drawing data with 'other' entry
        if (in_size > PIE_CHART_MAX_SECTIONS)
            ret++;
    }
    return ret;
}

// Set cairo draw color from a pie chart color array
static void pie_chart_set_color(cairo_t *cairo, PieChartColors *colors, int i)
{
    // Use 'other' color if section doesn't have designated color
    int j = i < PIE_CHART_MAX_SECTIONS ? i : PIE_CHART_MAX_SECTIONS;
    cairo_set_source_rgba(cairo,
                          colors[j].R,
                          colors[j].G,
                          colors[j].B,
                          colors[j].A);
}

// Setup global pie chart text drawing parameters
static void draw_pie_chart_text_setup(cairo_t *cairo, PieChartColors *colors)
{
    cairo_select_font_face(cairo,
                           PIE_CHART_FONT_FACE,
                           PIE_CHART_FONT_SLANT,
                           PIE_CHART_FONT_WEIGHT);
    cairo_set_font_size(cairo, PIE_CHART_FONT_SIZE);
    pie_chart_set_color(cairo, colors, PIE_CHART_MAX_SECTIONS);
}

// Draw pie chart text including crypto label and dollar balance
// Intended for donut charts for now
static void draw_pie_chart_text(cairo_t *cairo, cairo_text_extents_t *extents, CoinData *data, size_t size, PieChartColors *colors, double x, double y)
{
    appdata_s *ad = get_appdata(NULL);
    int lines = (int)size;
    // Vertical displacement between lines
    double dy = PIE_CHART_FONT_SIZE;
    // Vertical displacement to start drawing from
    double start_dy = (((double)lines - 1.0) * dy) / 2.0;
    // Add entries
    char *line;
    // Draw exchange label
    cairo_text_extents(cairo, ad->current_exchange, extents);
    cairo_set_source_rgba(cairo, RGBA_NORM(255, 255, 255, 255));
    cairo_move_to(cairo,
                  x - ((extents->width / 2.0) + extents->x_bearing),
                  y - ((extents->height / 2.0) + extents->y_bearing + (ad->height * 0.3)));
    cairo_show_text(cairo, ad->current_exchange);
    // Draw balances
    for (int i = 0; i < lines; i++)
    {
        // Set text color
        pie_chart_set_color(cairo, colors, i);
        // Get balance string to draw
        line = d_balance_str_fmt("",
                                 data[i].d_balance,
                                 PIE_CHART_MAX_BAL_WITH_DECIMAL,
                                 PIE_CHART_MAX_BAL_NON_SCIENTIFIC);
        // Draw balance
        cairo_text_extents(cairo, line, extents);
        cairo_move_to(cairo,
                      (2.0 * x) - (PIE_CHART_PRIMARY_LINE_WIDTH + 4.0 + extents->width + extents->x_bearing),
                      (y - start_dy) + (dy * (double)i) + (dy / 3.0));
        cairo_show_text(cairo, line);
        // Cleanup
        free(line);
        // Draw label
        cairo_text_extents(cairo, data[i].symbol, extents);
        cairo_move_to(cairo,
                      PIE_CHART_PRIMARY_LINE_WIDTH + 2.0,
                      (y - start_dy) + (dy * (double)i) + (dy / 3.0));
        cairo_show_text(cairo, data[i].symbol);
    }
    // Draw currency label
    cairo_text_extents(cairo, ad->currency, extents);
    cairo_set_source_rgba(cairo, RGBA_NORM(255, 255, 255, 255));
    cairo_move_to(cairo,
                  x - ((extents->width / 2.0) + extents->x_bearing),
                  y - ((extents->height / 2.0) + extents->y_bearing) + (ad->height * 0.3));
    cairo_show_text(cairo, ad->currency);
}

// Draw placeholder text when total balance is 0
static void draw_pie_chart_text_empty_balance(cairo_t *cairo, PieChartColors *colors, double x, double y)
{
    // Text to draw
    char *line = "NO DATA TO SHOW";
    // Setup text drawing
    draw_pie_chart_text_setup(cairo, colors);
    cairo_text_extents_t extents = {};
    cairo_text_extents(cairo, line, &extents);
    // Draw text centre-aligned
    cairo_move_to(cairo,
                  x - ((extents.width / 2.0) + extents.x_bearing),
                  y - ((extents.height / 2.0) + extents.y_bearing));
    cairo_show_text(cairo, line);
}

// Draw divider lines
static void draw_pie_chart_dividers(cairo_t *cairo, double x, double y)
{
    appdata_s *ad = get_appdata(NULL);
    // Line coordinates
    double x_left = PIE_CHART_PRIMARY_LINE_WIDTH + 4.3;
    double x_right = (2.0 * x) - (PIE_CHART_PRIMARY_LINE_WIDTH + 4.3);
    double y_top = y - (ad->height * 0.23);
    double y_bottom = y + (ad->height * 0.23);
    // Draw lines
    cairo_set_line_width(cairo, PIE_CHART_SECONDARY_LINE_WIDTH);
    cairo_set_source_rgba(cairo, RGBA_NORM(255, 255, 255, 255));
    // Top line
    cairo_move_to(cairo, x_left, y_top);
    cairo_line_to(cairo, x_right, y_top);
    // Bottom line
    cairo_move_to(cairo, x_left, y_bottom);
    cairo_line_to(cairo, x_right, y_bottom);
    // Update canvas
    cairo_stroke(cairo);
}

// Draw a donut pie chart with a hollow centre and text inside
static void draw_pie_chart_donut(appdata_s *ad, CoinData *in_data, size_t in_size, PieChartColors *colors)
{
    // Screen centre coordinates
    double x = ad->width / 2;
    double y = ad->height / 2;
    // Radius is smallest of half screen width / height
    double r = fmin(x, y);
    // Init drawing
    cairo_set_line_width(ad->cairo, PIE_CHART_PRIMARY_LINE_WIDTH);
    cairo_set_source_rgba(ad->cairo, PIE_CHART_BACKGROUND_COLOR);
    // Truncate data into 'other' section if necessary
    CoinData data[PIE_CHART_MAX_SECTIONS + 1] = {};
    int ret = pie_chart_data_truncate(in_data, (CoinData *)&data, in_size);
    // Number of sections to draw
    size_t size = (size_t)ret;
    // Get total balance
    double total_balance = 0.0;
    for (size_t i = 0; i < size; i++)
        total_balance += data[i].d_balance;
    // Skip drawing data if total balance is 0 and instead display placeholder text
    if (total_balance == 0.0)
    {
        draw_pie_chart_text_empty_balance(ad->cairo, colors, x, y);
        goto draw_pie_chart_donut_finish;
    }
    // Total angle to draw data for
    double total_angle = (2 * M_PI) - (size * PIE_CHART_SEPERATOR_LINE_THETA);
    // Angle to begin drawing at
    double current_angle = PIE_CHART_BEGIN_THETA;
    // Store end angle for each pass
    double end_angle;
    // Draw arc for each balance
    for (size_t i = 0; i < size; i++)
    {
        // Set arc color
        pie_chart_set_color(ad->cairo, colors, i);
        // Angle to draw until
        end_angle = current_angle + ((data[i].d_balance / total_balance) * total_angle);
        // Draw arc
        cairo_arc(ad->cairo, x, y, r, current_angle, end_angle);
        cairo_stroke(ad->cairo);
        // Increment angle by line seperator
        current_angle = end_angle + PIE_CHART_SEPERATOR_LINE_THETA;
    }
    // Restart drawing from beginning
    current_angle = PIE_CHART_BEGIN_THETA;
    // Rectangle vertices
    double theta, a_x, a_y, b_x, b_y, c_x, c_y, d_x, d_y;
    // Paint rectangles in background color
    cairo_set_source_rgba(ad->cairo, PIE_CHART_BACKGROUND_COLOR);
    // Draw rectangles in seperator regions
    for (size_t i = 0; i < size; i++)
    {
        // Angle arc goes to
        end_angle = current_angle + ((data[i].d_balance / total_balance) * total_angle);
        // Angle to draw rectangle width
        theta = (M_PI / 2) - end_angle;
        // Rectangle vertices
        a_x = x + (r * cos(end_angle));
        a_y = y + (r * sin(end_angle));
        b_x = x + (r * cos(end_angle + PIE_CHART_SEPERATOR_LINE_THETA));
        b_y = y + (r * sin(end_angle + PIE_CHART_SEPERATOR_LINE_THETA));
        c_x = b_x + (PIE_CHART_PRIMARY_LINE_WIDTH * cos(theta));
        c_y = b_y + (PIE_CHART_PRIMARY_LINE_WIDTH * sin(theta));
        d_x = a_x + (PIE_CHART_PRIMARY_LINE_WIDTH * cos(theta));
        d_y = a_y + (PIE_CHART_PRIMARY_LINE_WIDTH * sin(theta));
        // Draw rectangle
        cairo_move_to(ad->cairo, a_x, a_y);
        cairo_line_to(ad->cairo, b_x, b_y);
        cairo_stroke(ad->cairo);
        cairo_line_to(ad->cairo, c_x, c_y);
        cairo_stroke(ad->cairo);
        cairo_line_to(ad->cairo, d_x, d_y);
        cairo_stroke(ad->cairo);
        cairo_line_to(ad->cairo, a_x, a_y);
        cairo_stroke(ad->cairo);
        cairo_fill(ad->cairo);
        // Go to next arc
        current_angle = end_angle + PIE_CHART_SEPERATOR_LINE_THETA;
    }
    // Setup text drawing
    cairo_text_extents_t extents = {};
    draw_pie_chart_text_setup(ad->cairo, colors);
    // Draw pie chart text
    draw_pie_chart_text(ad->cairo, &extents, (CoinData *)&data, size, colors, x, y);
    // Draw divider lines
    draw_pie_chart_dividers(ad->cairo, x, y);
draw_pie_chart_donut_finish:
    // Finish drawing and update image data
    cairo_image_update(ad);
}

// Show a pie chart based on crypto balance data
void show_pie_chart_view(appdata_s *ad, CoinData *data, size_t size, PieChartColors *colors, int style)
{
    // Init cairo and image
    cairo_image_new(ad);
    // Draw chart
    switch (style)
    {
    case PIE_CHART_STYLE_DONUT:
        draw_pie_chart_donut(ad, data, size, colors);
        break;
    }
    // Push view to naviframe
    Elm_Object_Item *nf_it = NULL;
    nf_it = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->image, "empty");
    elm_naviframe_item_pop_cb_set(nf_it, (Elm_Naviframe_Item_Pop_Cb)pie_chart_pop_cb, ad);
}

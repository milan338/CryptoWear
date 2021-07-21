#ifndef PIECHARTVIEW_H
#define PIECHARTVIEW_H

#include <cairo.h>
#include "main.h"

// Line style
#define PIE_CHART_PRIMARY_LINE_WIDTH 50.0
#define PIE_CHART_SECONDARY_LINE_WIDTH 2.0
#define PIE_CHART_SEPERATOR_LINE_THETA (M_PI / 96.0)
#define PIE_CHART_BACKGROUND_COLOR RGBA_NORM(0, 0, 0, 255)
// Where to start drawing arc
#define PIE_CHART_BEGIN_THETA ((3.0 / 2.0) * M_PI) + (PIE_CHART_SEPERATOR_LINE_THETA / 2.0)
// Chart styles
#define PIE_CHART_STYLE_PIE 0
#define PIE_CHART_STYLE_DONUT 1
// Data visualisation
#define PIE_CHART_MAX_SECTIONS 4
#define PIE_CHART_MAX_BAL_WITH_DECIMAL 100000
#define PIE_CHART_MAX_BAL_NON_SCIENTIFIC 1000000000
// Text style
#define PIE_CHART_FONT_FACE "Sans"
#define PIE_CHART_FONT_SLANT CAIRO_FONT_SLANT_NORMAL
#define PIE_CHART_FONT_WEIGHT CAIRO_FONT_WEIGHT_NORMAL
#define PIE_CHART_FONT_SIZE 32.0
#define PIE_CHART_SYMBOL_SIZE 10.0

typedef struct PieChartColors
{
    double R;
    double G;
    double B;
    double A;
} PieChartColors;

void show_pie_chart_view(appdata_s *ad, CoinData *data, size_t size, PieChartColors *colors, int style);

#endif // PIECHARTVIEW_H

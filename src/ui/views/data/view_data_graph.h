#ifndef VIEWDATAGRAPH_H
#define VIEWDATAGRAPH_H

#include "main.h"

typedef struct TimelineData
{
    char name[9];
    char *data;
} TimelineData;

static const char *graph_html_template =
    "<div class=\"tradingview-widget-copyright\""
    "style= \""
    "margin: 0;"
    "position: absolute;"
    "top: 10%%;"
    "left: 50%%;"
    "transform: translate(-50%%, -50%%) scale(0.85);"
    "white-space: nowrap;"
    "overflow: hidden;\">"
    "%s%s Rates by TradingView"
    "</div>"
    "<body style=\"background-color: rgb(0, 0, 0)\">"
    "<div class=\"tradingview-widget-container\""
    "style= \""
    "margin: 0;"
    "position: absolute;"
    "top: 50%%;"
    "left: 50%%;"
    "transform: translate(-50%%, -50%%) scale(0.82, 0.82);\">"
    "<script type=\"text/javascript\""
    "src=\"https://s3.tradingview.com/external-embedding/embed-widget-mini-symbol-overview.js\""
    "async>"
    "{"
    "\"symbol\": \"%s:%s%s\","
    "\"width\": \"%d\","
    "\"height\": \"%d\","
    "\"locale\": \"en\","
    "\"dateRange\": \"%s\","
    "\"colorTheme\": \"dark\","
    "\"trendLineColor\": \"rgba(55, 166, 239, 1)\","
    "\"underLineColor\": \"rgba(0, 0, 0, 1)\","
    "\"isTransparent\": true,"
    "\"autosize\": true,"
    "\"largeChartUrl\": \"\""
    "}"
    "</script>"
    "</div>"
    "</body>";

void view_data_graph(int i);

void timeline_more_option_closed_cb(void *data, Evas_Object *obj, void *event_info);

#endif // VIEWDATAGRAPH_H

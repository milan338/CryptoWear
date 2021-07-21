#ifndef ARTICLEVIEW_H
#define ARTICLEVIEW_H

#include "main.h"

static const char *article_html_template =
    "<body style= \""
    "font-family: 'TizenSans', sans-serif;"
    "text-align: justify;"
    "background-color: black;"
    "color: white;\">"
    "<div style=\"height: 50px\"></div>"
    "<div style=\"margin-left: 30px; margin-right: 30px\">"
    "<h4 style=\"margin-bottom: 10px\">%s</h4>"      // Title
    "<h5 style=\"margin: 0px\">%s</h5>"              // Publisher
    "<h5 style=\"margin: 0px; color: grey\">%s</h5>" // Date
    "<p style=\"margin: 0px\">%s</p>"                // Body
    "</div>"
    "<div style=\"height: 20px\"></div>"
    "<div style=\"text-align: center\">"
    "<button style= \""
    "border: none;"
    "outline: none;"
    "padding: 15px 35px 15px 35px;"
    "border-radius: 30px;"
    "background-color: #002a4d;"
    "color: #38a4fc;"
    "font-weight: bold;\""
    "onclick=\"window.open('%s', '_self')\">" // Article url
    "Read the full article"
    "</button>"
    "</div>"
    "<div style=\"height: 40px\"></div>"
    "</body>";

void show_article_view(CoinNewsData *news);

#endif // ARTICLEVIEW_H

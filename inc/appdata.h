#ifndef APPDATA_H
#define APPDATA_H

#include <bundle.h>
#include <cairo.h>
#include "../src/thread/lock.h"
#include "main.h"

typedef struct appdata
{
    double last_updated;
    char currency[4];
    char data_provider[20];
    char *current_exchange;
    char *proxy_addr;
    int width;
    int height;
    bool win_lower;
    bool enable_pop;
    Evas_Object *win;
    Evas_Object *conform;
    Evas_Object *naviframe;
    Evas_Object *image;
    Evas_Object *web_view;
    Evas_Object *web_view_btn;
    Evas_Object *popup;
    Evas_Object *progressbar;
    Evas_Object *exchanges_it;
    Evas_Object *balances_it;
    Evas_Object *more_option;
    Evas_Object *genlist;
    Evas_Object *circle_genlist;
    Evas_Object *circle_progressbar;
    Evas_Object *radio_group;
    Evas_Object *entry;
    Eext_Circle_Surface *circle_suface;
    Elm_Genlist_Item_Class genlist_line_class;
    Elm_Genlist_Item_Class genlist_radio_class;
    Elm_Genlist_Item_Class genlist_setting_class;
    Elm_Genlist_Item_Class genlist_refresh_class;
    Elm_Genlist_Item_Class genlist_crypto_class;
    Elm_Genlist_Item_Class genlist_fiat_class;
    Elm_Genlist_Item_Class genlist_news_class;
    Elm_Genlist_Item_Class genlist_title_class;
    Elm_Genlist_Item_Class genlist_groupindex_class;
    Elm_Genlist_Item_Class genlist_padding_class;
    Elm_Widget_Item *refresh_it;
    cairo_t *cairo;
    cairo_surface_t *cairo_surface;
    CoinData *coin_data;
    size_t coin_data_size;
    bundle *crypto_names;
    bundle *exchange_data;
    bundle *fiat_data;
    bundle *coin_price_data;
    bundle *coin_news;
    bundle *icon_urls;
    bundle *file_status;
    Eina_Strbuf *crypto_to_convert;
    Lock file_status_mutex;
    Lock coin_list_mutex;
    Lock fiat_data_mutex;
    Lock coin_price_data_mutex;
    Lock coin_news_mutex;
    Lock icon_urls_mutex;
} appdata_s;

#endif // APPDATA_H

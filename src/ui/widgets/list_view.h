#ifndef LISTVIEW_H
#define LISTVIEW_H

#include "main.h"

void genlist_title_add(appdata_s *ad, const char *label);
void genlist_groupindex_add(appdata_s *ad, const char *label);
void genlist_line_add(appdata_s *ad, const char *label, void *callback, void *cb_data);
void genlist_news_add(appdata_s *ad, CoinNewsData *data, void *callback, void *cb_data);
void genlist_radio_add(appdata_s *ad, const char *label, int goto_hash, void *callback, void *cb_data);
Elm_Widget_Item *genlist_setting_add(appdata_s *ad, const char *label, void *callback, void *cb_data);
Elm_Widget_Item *genlist_refresh_add(appdata_s *ad, const char *label, void *callback, void *cb_data);
void genlist_crypto_add(appdata_s *ad, const char *label, void *callback, void *cb_data);
void genlist_fiat_add(appdata_s *ad, const char *label, int goto_hash, void *callback, void *cb_data);
void genlist_padding_add(appdata_s *ad);

void create_genlist_item_classes(appdata_s *ad);
Elm_Object_Item *show_list_view(appdata_s *ad);

#endif // LISTVIEW_H

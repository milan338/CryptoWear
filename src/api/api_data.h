#ifndef APIDATA_H
#define APIDATA_H

#include <bundle.h>
#include "main.h"

Eina_Bool coin_data_cleanup_cb(const void *container, void *data, void *fdata);
void exchange_data_bundle_cleanup_cb(const char *key, const int type, bundle_keyval_t *kv, void *user_data);
void fiat_data_bundle_cleanup_cb(const char *key, const int type, bundle_keyval_t *kv, void *user_data);
void coin_news_bundle_cleanup_cb(const char *key, const int type, bundle_keyval_t *kv, void *user_data);

extern int array_iter_index;
Eina_Bool coin_data_copy_cb(const void *container, void *data, void *fdata);

void exchange_data_clear(bool show_confirm);

void eina_array_coin_data_quicksort(Eina_Array *array, int start_i, int end_i);

void coin_data_parse_data(bundle *b, Eina_Array *array);
void coin_data_bundle_append(bundle *b, CoinData *data);

#endif // APIDATA_H

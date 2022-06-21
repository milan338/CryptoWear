#include <bundle.h>
#include <json-glib/json-glib.h>
#include "main.h"
#include "../ui/widgets/popup_view.h"
#include "../rw/json.h"
#include "api_base.h"
#include "api_data.h"

int array_iter_index;
JsonParser *json_parser = NULL;
JsonObject *coin_names = NULL;

// Callback function to free data from a CoinData struct from an Eina_Array
Eina_Bool coin_data_cleanup_cb(const void *container, void *data, void *fdata)
{
    CoinData *coin_data = data;
    free(coin_data->symbol);
    return EINA_TRUE;
}

// Called to free elements of an ExchangeData struct from a bundle
void exchange_data_bundle_cleanup_cb(const char *key, const int type, bundle_keyval_t *kv, void *user_data)
{
    ExchangeData *data = NULL;
    size_t size;
    bundle_keyval_get_basic_val(kv, (void **)&data, &size);
    // Free struct elements
    if (data->coin_data_sorted != NULL)
    {
        // Cleanup free sorted CoinData array
        eina_array_foreach(data->coin_data_sorted, coin_data_cleanup_cb, NULL);
        eina_array_cleanup(data->coin_data_sorted);
        // Cleanup CoinData dict - shared with array, so is not freed again
        bundle_free(data->coin_data_dict);
    }
    // Don't free parent struct - should be freed by bundle
}

// Called to free elements of a FiatData struct from a bundle
void fiat_data_bundle_cleanup_cb(const char *key, const int type, bundle_keyval_t *kv, void *user_data)
{
    FiatData *data = NULL;
    size_t size;
    bundle_keyval_get_basic_val(kv, (void **)&data, &size);
    // Free struct elements
    if (data->name != NULL)
        free(data->name);
    if (data->symbol != NULL)
        free(data->symbol);
    // Don't free parent struct - should be freed by bundle
}

// Called to free elements of a CoinNews struct from a bundle
void coin_news_bundle_cleanup_cb(const char *key, const int type, bundle_keyval_t *kv, void *user_data)
{
    CoinNews *data = NULL;
    size_t size;
    bundle_keyval_get_basic_val(kv, (void **)&data, &size);
    // Get CoinNewsData array
    CoinNewsData **arr = data->news_arr;
    // Free array elements and contained struct elements
    for (size_t i = 0; i < data->size; i++)
    {
        CoinNewsData *news = arr[i];
        free(news->source);
        free(news->url);
        free(news->title);
        free(news->body);
        free(news);
    }
    // Free array
    free(arr);
    // Don't free parent struct - should be freed by bundle
}

void free_coin_data_json_parser()
{
    if (json_parser != NULL)
        g_object_unref(json_parser);
}

// Copy contents from CoinData struct in Eina_Array to CoinData array
// Reset array_iter_index to 0 before execution
Eina_Bool coin_data_copy_cb(const void *container, void *data, void *fdata)
{
    appdata_s *ad = get_appdata(NULL);
    // Create new json parser that lasts app lifetime
    if (json_parser == NULL && ad->crypto_names_data != NULL)
    {
        json_parser = json_parse_new_parser();
        GError *err = NULL;
        json_load_data(json_parser, ad->crypto_names_data, JSON_DATA_TYPE_MEMORY, err);
        JsonNode *json_root = json_parser_get_root(json_parser);
        JsonObject *root_obj = json_node_get_object(json_root);
        coin_names = json_object_get_object_member(root_obj, "Data");
        // Cleanup - g_object_unref called in main at app cleanup
        g_error_free(err);
    }
    CoinData *in_data = data;
    CoinData *out_data = fdata;
    // Copy data
    out_data[array_iter_index].c_balance = in_data->c_balance;
    out_data[array_iter_index].d_balance = in_data->d_balance;
    // Since the original struct still exists in the array, and this is temporary copy,
    // The string pointer can still point to the same memory location
    out_data[array_iter_index].symbol = in_data->symbol;
    // Download icon for image if it isn't already cached
    download_crypto_icon(out_data[array_iter_index].symbol);
    // Get coin name from json
    if (coin_names != NULL)
    {
        JsonObject *coin_object = json_object_get_object_member(coin_names, in_data->symbol);
        // const char *symbol = json_object_get_string_member(coin_names, "Symbol");
        const char *full_name = json_object_get_string_member(coin_object, "FullName");
        // full_name in format "Coin Name (SYMBOL)", so need to remove symbol
        int symbol_len = 1;
        for (int i = strlen(full_name) - 1; i >= 0; i--)
        {
            if (full_name[i] == '(')
                break;
            symbol_len++;
        }
        char *name = strndup(full_name, strlen(full_name) - symbol_len);
        lock_take(&ad->coin_list_mutex);
        bundle_add_str(ad->crypto_names, in_data->symbol, name);
        lock_release(&ad->coin_list_mutex);
        // Cleanup
        free(name);
    }
    // Copy next data
    array_iter_index++;
    return EINA_TRUE;
}

// Clear entire exchange data bundle
void exchange_data_clear(bool show_confirm)
{
    appdata_s *ad = get_appdata(NULL);
    // Clear exchange data
    bundle_foreach(ad->exchange_data, (bundle_iterator_t)exchange_data_bundle_cleanup_cb, NULL);
    bundle_free(ad->exchange_data);
    ad->exchange_data = bundle_create();
    // Update last updated time
    ad->last_updated = ecore_time_unix_get();
    // Update last refreshed time
    elm_genlist_item_fields_update(ad->refresh_it, "elm.text.1", ELM_GENLIST_ITEM_FIELD_TEXT);
    // Show popup confirmation
    if (show_confirm)
        show_popup_view_ok(ad, "Refresh Data", "Data has been refreshed.", elm_naviframe_item_pop, ad->naviframe);
}

// Swap two CoinData structs
static void eina_array_coin_data_swap(CoinData *data_a, CoinData *data_b)
{
    CoinData tmp = *data_a;
    *data_a = *data_b;
    *data_b = tmp;
}

// Partition Eina_Array of CoinData structs based on dollar balance and a pivot
static int eina_array_coin_data_partition(Eina_Array *array, int start_i, int end_i)
{
    // Get pivot
    CoinData *pivot = eina_array_data_get(array, end_i);
    int i = start_i - 1;
    // CoinData with dollar balance greater than that of the pivot on the left, less than on the right
    for (int j = start_i; j < end_i; j++)
    {
        CoinData *data_j = eina_array_data_get(array, j);
        if (data_j->d_balance >= pivot->d_balance)
        {
            i++;
            eina_array_coin_data_swap(eina_array_data_get(array, i), data_j);
        }
    }
    eina_array_coin_data_swap(
        eina_array_data_get(array, i + 1),
        eina_array_data_get(array, end_i));
    return i + 1;
}

// Sort CoinData structs in Eina_Array based on dollar balance, greatest - smallest
void eina_array_coin_data_quicksort(Eina_Array *array, int start_i, int end_i)
{
    if (start_i >= end_i)
        return;
    // Select pivot position
    int pivot_i = eina_array_coin_data_partition(array, start_i, end_i);
    // Sort elements left of the pivot
    eina_array_coin_data_quicksort(array, start_i, pivot_i - 1);
    // Sort elements right of the pivot
    eina_array_coin_data_quicksort(array, pivot_i + 1, end_i);
}

// Copy data from bundle to Eina_Array
static void coin_data_parse_data_cb(const char *key, const int type, bundle_keyval_t *kv, void *user_data)
{
    // Get CoinData from bundle
    CoinData *data = NULL;
    size_t size;
    bundle_keyval_get_basic_val(kv, (void **)&data, &size);
    // data->symbol is not reallocated, since it will not be freed in this process
    CoinData *coin_data = malloc(sizeof(CoinData));
    // memcpy(coin_data, data, sizeof(CoinData));
    *coin_data = *data;
    // Push data to array
    Eina_Array *array = user_data;
    eina_array_push(array, coin_data);
}

// Parse raw CoinData structs in bundle from api into Eina_Array and sort
void coin_data_parse_data(bundle *b, Eina_Array *array)
{
    // Copy data from bundle to Eina_Array
    bundle_foreach(b, (bundle_iterator_t)coin_data_parse_data_cb, array);
    // Sort Eina_Array coin data by dollar value
    eina_array_coin_data_quicksort(array, 0, eina_array_count(array) - 1);
}

// Append CoinData to temporary bundle, merging balances for duplicate entries
void coin_data_bundle_append(bundle *b, CoinData *data)
{
    CoinData *bundle_data;
    size_t size;
    int ret = bundle_get_byte(b, data->symbol, (void **)&bundle_data, &size);
    // Struct not already in bundle
    if (ret != BUNDLE_ERROR_NONE)
        // Add CoinData struct to bundle
        bundle_add_byte(b, data->symbol, data, sizeof(*data));
    // Struct already exists in bundle
    else
    {
        // Add to existing coin balance and dollar balance
        bundle_data->c_balance += data->c_balance;
        bundle_data->d_balance += data->d_balance;
        free(data->symbol);
        free(data);
    }
}

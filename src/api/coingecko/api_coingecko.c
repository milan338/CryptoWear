#include <curl/curl.h>
#include <json-glib/json-glib.h>
#include <bundle.h>
#include "../api_base.h"
#include "../api_data.h"
#include "../../rw/json.h"
#include "../../rw/web.h"
#include "../../rw/file.h"
#include "main.h"
#include "api_coingecko.h"

// Add coin list data to bundle
static void api_coingecko_coin_list_parse_json_array_cb(JsonArray *array, guint index, JsonNode *member_node, gpointer user_data)
{
    bundle *b = user_data;
    JsonObject *obj = json_node_get_object(member_node);
    char *symbol = (char *)json_object_get_string_member(obj, "symbol");
    const char *name = json_object_get_string_member(obj, "name");
    // Ignore binance-peg variants
    if (strstr(name, "Binance-Peg") == NULL)
    {
        eina_str_toupper(&symbol);
        bundle_add_str(b, symbol, name);
    }
}

// Parse json response from coingecko coin list and dump into bundle
static void api_coingecko_coin_list_parse_json(appdata_s *ad, MemoryStruct *data)
{
    JsonParser *json_parser = json_parse_new_parser();
    GError *err = NULL;
    // Load data
    json_load_data(json_parser, data, JSON_DATA_TYPE_MEMORY, err);
    // Parse data
    JsonNode *json_root = json_parser_get_root(json_parser);
    JsonArray *root_array = json_node_get_array(json_root);
    // Load data from array members
    eina_lock_take(&ad->coin_list_mutex);
    json_array_foreach_element(root_array, api_coingecko_coin_list_parse_json_array_cb, ad->crypto_names);
    eina_lock_release(&ad->coin_list_mutex);
    // Cleanup
    g_error_free(err);
    g_object_unref(json_parser);
}

// Get a list of crypto symbols and their full names and cache it to a local json file
void api_coingecko_coin_list(appdata_s *ad)
{
    eina_lock_take(&ad->coin_list_mutex);
    // Init bundle for storing json data
    if (ad->crypto_names != NULL)
        bundle_free(ad->crypto_names);
    ad->crypto_names = bundle_create();
    eina_lock_release(&ad->coin_list_mutex);
    // send api request
    MemoryStruct *chunk = web_request_write_memory(API_COINGECKO_COIN_LIST, NULL, NULL, REQUEST_TYPE_GET);
    // Request success
    if (chunk != NULL)
    {
        // Parse response
        api_coingecko_coin_list_parse_json(ad, chunk);
        // Cleanup
        web_request_cleanup(chunk);
    }
}

// Get crypto names from a thread
void api_coingecko_coin_list_thread_cb(void *data, Ecore_Thread *thread)
{
    appdata_s *ad = data;
    api_coingecko_coin_list(ad);
}

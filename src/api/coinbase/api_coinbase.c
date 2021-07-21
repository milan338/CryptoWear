#include <curl/curl.h>
#include <json-glib/json-glib.h>
#include <bundle.h>
#include "main.h"
#include "../api_base.h"
#include "../api_data.h"
#include "../cryptocompare/api_cryptocompare.h"
#include "../../rw/json.h"
#include "../../rw/web.h"
#include "../../rw/key.h"
#include "api_coinbase.h"

// Callback to parse the 'data' array in the api response json
static void api_coinbase_parse_data_cb(JsonArray *array, guint index, JsonNode *member_node, gpointer user_data)
{
    appdata_s *ad = get_appdata(NULL);
    bundle *coin_data = user_data;
    // Individual crypto balance entry within array
    JsonObject *obj = json_node_get_object(member_node);
    JsonObject *balance = json_object_get_object_member(obj, "balance");
    // Ignore empty balances
    if (atof(json_object_get_string_member(balance, "amount")) == 0.0)
        return;
    CoinData *data = malloc(sizeof(CoinData));
    data->symbol = strdup(json_object_get_string_member(balance, "currency"));
    // The balance is received as a string, so must be converted to a double
    data->c_balance = atof(json_object_get_string_member(balance, "amount"));
    data->d_balance = 0.0;
    // Add CoinData struct to temporary bundle
    coin_data_bundle_append(coin_data, data);
    // Mark crypto for conversion
    eina_strbuf_append_printf(ad->crypto_to_convert, "%s,", data->symbol);
}

// Parse json api response from coinbase
// Returned value must be freed if not null
static char *api_coinbase_parse_json(appdata_s *ad, MemoryStruct *data, bundle *coin_data)
{
    char *ret = NULL;
    JsonParser *json_parser = json_parse_new_parser();
    GError *err = NULL;
    // Load data
    json_load_data(json_parser, data, JSON_DATA_TYPE_MEMORY, err);
    // Parse data
    JsonNode *json_root = json_parser_get_root(json_parser);
    JsonObject *root_obj = json_node_get_object(json_root);
    JsonArray *balances = json_object_get_array_member(root_obj, "data");
    json_array_foreach_element(balances, api_coinbase_parse_data_cb, coin_data);
    // Check for additional pages
    JsonObject *pagination_obj = json_object_get_object_member(root_obj, "pagination");
    const char *next_uri = json_object_get_string_member(pagination_obj, "next_uri");
    // Another page exists, re-run api for this new page
    if (next_uri != NULL)
        ret = strdup(next_uri);
    // Cleanup
    g_error_free(err);
    g_object_unref(json_parser);
    return ret;
}

// Get single page from coinbase
static void api_coinbase_get_page(appdata_s *ad, char *request_path, bundle *coin_data, int *flag)
{
    // Get api keys
    char *key = (char *)get_secure_data("coinbase_key");
    char *secret = (char *)get_secure_data("coinbase_secret");
    // Generate signed header
    int timestamp = (int)ecore_time_unix_get();
    char *method = "GET";
    char *api_version = API_COINBASE_VERSION;
    char *url = new_string_printf(API_COINBASE_URL_BASE "%s", request_path);
    char *access_sign_prehash = new_string_printf("%d%s%s", timestamp, method, request_path);
    char *access_sign_hash = sign_hmac_string(access_sign_prehash, secret, G_CHECKSUM_SHA256);
    // Http headers
    char *cb_access_key = new_string_printf("CB-ACCESS-KEY:%s", key);
    char *cb_access_sign = new_string_printf("CB-ACCESS-SIGN:%s", access_sign_hash);
    char *cb_access_timestamp = new_string_printf("CB-ACCESS-TIMESTAMP:%d", timestamp);
    char *cb_version = new_string_printf("CB-VERSION:%s", api_version);
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-type:application/json");
    headers = curl_slist_append(headers, "Accept:text/plain");
    headers = curl_slist_append(headers, cb_access_key);
    headers = curl_slist_append(headers, cb_access_sign);
    headers = curl_slist_append(headers, cb_access_timestamp);
    headers = curl_slist_append(headers, cb_version);
    // Send api request
    MemoryStruct *chunk = web_request_write_memory(url, NULL, headers, REQUEST_TYPE_GET);
    char *next_uri = NULL;
    // Request success
    if (chunk != NULL)
    {
        // Parse response into ExchangeData
        next_uri = api_coinbase_parse_json(ad, chunk, coin_data);
        // Cleanup
        web_request_cleanup(chunk);
    }
    // Cleanup
    g_free(access_sign_hash);
    free(access_sign_prehash);
    free(cb_access_key);
    free(cb_access_sign);
    free(cb_access_timestamp);
    free(cb_version);
    free(url);
    free(key);
    free(secret);
    curl_slist_free_all(headers);
    // Request failed
    if (chunk == NULL)
    {
        // Mark thread for cancellation
        *flag = 1;
        return;
    }
    // Get data from other pages
    else if (next_uri != NULL)
    {
        api_coinbase_get_page(ad, next_uri, coin_data, flag);
        free(next_uri);
    }
}

// Get balance data from coinbase
// Don't call if data already exists
void api_coinbase(void *data, Ecore_Thread *thread)
{
    appdata_s *ad = get_appdata(NULL);
    // Check if data already exists in bundle
    ExchangeData *coinbase_data = NULL;
    size_t coinbase_size;
    int ret = bundle_get_byte(ad->exchange_data, "coinbase", (void **)&coinbase_data, &coinbase_size);
    // Init data
    ad->crypto_to_convert = eina_strbuf_new();
    if (ret != BUNDLE_ERROR_NONE)
    {
        // Create new struct
        coinbase_data = malloc(sizeof(ExchangeData));
        coinbase_data->coin_data_sorted = eina_array_new(3);
        coinbase_data->coin_data_dict = bundle_create();
        // Push new struct to bundle
        bundle_add_byte(ad->exchange_data, "coinbase", coinbase_data, sizeof(*coinbase_data));
    }
    // Get data from coinbase
    int flag = 0;
    api_coinbase_get_page(ad, API_COINBASE_PATH_BASE, coinbase_data->coin_data_dict, &flag);
    // Thread marked for cancellation
    if (flag == 1)
    {
        // Remove any existing bundle data
        ret = bundle_get_byte(ad->exchange_data, "coinbase", (void **)&coinbase_data, &coinbase_size);
        // Coinbase data exists, free resources
        if (ret == BUNDLE_ERROR_NONE)
        {
            eina_array_foreach(coinbase_data->coin_data_sorted, coin_data_cleanup_cb, NULL);
            eina_array_cleanup(coinbase_data->coin_data_sorted);
            bundle_free(coinbase_data->coin_data_dict);
            bundle_del(ad->exchange_data, "coinbase");
        }
        // Cleanup
        eina_strbuf_free(ad->crypto_to_convert);
        // Cancel the current thread immediately
        ecore_thread_cancel(thread);
        return;
    }
    // Get dollar balances for each crypto
    char *from = eina_strbuf_string_steal(ad->crypto_to_convert);
    api_cryptocompare_full_sync(from, coinbase_data->coin_data_dict);
    // Parse data from bundle into Eina_Array
    coin_data_parse_data(coinbase_data->coin_data_dict, coinbase_data->coin_data_sorted);
    // Sort CoinData structs based on dollar balance
    eina_array_coin_data_quicksort(coinbase_data->coin_data_sorted, 0, eina_array_count(coinbase_data->coin_data_sorted) - 1);
    // Cleanup
    free(from);
    eina_strbuf_free(ad->crypto_to_convert);
}

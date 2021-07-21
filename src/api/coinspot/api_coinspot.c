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
#include "api_coinspot.h"

// Callback to parse each individual balance from the 'balances' array in the api response json
static void api_coinspot_parse_balances_parse_obj_cb(JsonObject *object, const gchar *member_name, JsonNode *member_node, gpointer user_data)
{
    appdata_s *ad = get_appdata(NULL);
    bundle *coin_data = user_data;
    JsonObject *obj = json_node_get_object(member_node);
    // Ignore empty balances
    if (json_object_get_double_member(obj, "balance") == 0.0)
        return;
    CoinData *data = malloc(sizeof(CoinData));
    // Coin symbol - member name
    data->symbol = strdup(member_name);
    data->c_balance = json_object_get_double_member(obj, "balance");
    data->d_balance = 0.0;
    // Add CoinData struct to temporary bundle
    coin_data_bundle_append(coin_data, data);
    // Mark crypto for conversion
    eina_strbuf_append_printf(ad->crypto_to_convert, "%s,", data->symbol);
}

// Callback to parse the 'balances' array in the api response json
static void api_coinspot_parse_balances_cb(JsonArray *array, guint index, JsonNode *member_node, gpointer user_data)
{
    bundle *coin_data = user_data;
    // Individual crypto balance entry within array
    JsonObject *obj = json_node_get_object(member_node);
    // Get all crypto balances within this obj - should be only one
    json_object_foreach_member(obj, api_coinspot_parse_balances_parse_obj_cb, coin_data);
}

// Parse json api response from coinspot
static void api_coinspot_parse_json(appdata_s *ad, MemoryStruct *data, ExchangeData *coinspot_data)
{
    JsonParser *json_parser = json_parse_new_parser();
    GError *err = NULL;
    // Load data
    json_load_data(json_parser, data, JSON_DATA_TYPE_MEMORY, err);
    // Parse data
    JsonNode *json_root = json_parser_get_root(json_parser);
    JsonObject *root_obj = json_node_get_object(json_root);
    JsonArray *balances = json_object_get_array_member(root_obj, "balances");
    ad->crypto_to_convert = eina_strbuf_new();
    json_array_foreach_element(balances, api_coinspot_parse_balances_cb, coinspot_data->coin_data_dict);
    // Get dollar balances for each crypto
    char *from = eina_strbuf_string_steal(ad->crypto_to_convert);
    api_cryptocompare_full_sync(from, coinspot_data->coin_data_dict);
    // Parse data from bundle into Eina_Array
    coin_data_parse_data(coinspot_data->coin_data_dict, coinspot_data->coin_data_sorted);
    // Cleanup
    free(from);
    eina_strbuf_free(ad->crypto_to_convert);
    g_error_free(err);
    g_object_unref(json_parser);
}

// Get balance data from coinspot
// Don't call if data already exists
void api_coinspot(void *data, Ecore_Thread *thread)
{
    appdata_s *ad = get_appdata(NULL);
    // Check if data already exists in bundle
    ExchangeData *coinspot_data = NULL;
    size_t coinspot_size;
    int ret = bundle_get_byte(ad->exchange_data, "coinspot", (void **)&coinspot_data, &coinspot_size);
    // Get api keys
    char *key = (char *)get_secure_data("coinspot_key");
    char *secret = (char *)get_secure_data("coinspot_secret");
    // Http headers
    char *key_header = new_string_printf("key:%s", key);
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-type:application/json");
    headers = curl_slist_append(headers, "Accept:text/plain");
    headers = curl_slist_append(headers, key_header);
    // Generate unique nonce
    unsigned long long nonce = get_unix_time_ms();
    // POST data
    char *post_data = new_string_printf("{\"nonce\":%llu}", nonce);
    char *post_data_signed = sign_hmac_string(post_data, secret, G_CHECKSUM_SHA512);
    // Signed http header
    char *sign_header = new_string_printf("sign:%s", post_data_signed);
    headers = curl_slist_append(headers, sign_header);
    // Send api request
    MemoryStruct *chunk = web_request_write_memory("https://www.coinspot.com.au/api/ro/my/balances", post_data, headers, REQUEST_TYPE_POST);
    // Cleanup
    g_free(post_data_signed);
    free(sign_header);
    free(post_data);
    free(key);
    free(key_header);
    free(secret);
    curl_slist_free_all(headers);
    // Data doesn't already exists
    if (ret != BUNDLE_ERROR_NONE)
    {
        coinspot_data = malloc(sizeof(ExchangeData));
        coinspot_data->coin_data_sorted = eina_array_new(3);
        coinspot_data->coin_data_dict = bundle_create();
    }
    // Request success
    if (chunk != NULL)
    {
        // Parse response into ExchangeData
        api_coinspot_parse_json(ad, chunk, coinspot_data);
        // Data doesn't exist
        if (ret != BUNDLE_ERROR_NONE)
            // Push parsed data into main bundle
            bundle_add_byte(ad->exchange_data, "coinspot", coinspot_data, sizeof(*coinspot_data));
        // Cleanup
        web_request_cleanup(chunk);
    }
    // Request failed
    else
    {
        ecore_thread_cancel(thread);
        return;
    }
}

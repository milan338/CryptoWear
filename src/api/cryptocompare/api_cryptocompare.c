#include <curl/curl.h>
#include <json-glib/json-glib.h>
#include <bundle.h>
#include "main.h"
#include "../../rw/web.h"
#include "../../rw/json.h"
#include "api_cryptocompare.h"

static void api_cryptocompare_news_parse_json_array_cb(JsonArray *array, guint index, JsonNode *member_node, gpointer user_data)
{
    CoinNewsData **news_arr = (CoinNewsData **)user_data;
    JsonObject *obj = json_node_get_object(member_node);
    JsonObject *source_info = json_object_get_object_member(obj, "source_info");
    CoinNewsData *news = malloc(sizeof(CoinNewsData));
    news->published_on = json_object_get_int_member(obj, "published_on");
    news->source = strdup(json_object_get_string_member(source_info, "name"));
    news->url = strdup(json_object_get_string_member(obj, "url"));
    news->title = strdup(json_object_get_string_member(obj, "title"));
    news->body = strdup(json_object_get_string_member(obj, "body"));
    news_arr[index] = news;
}

// Parse json api response from news api
static void api_cryptocompare_news_parse_json(appdata_s *ad, MemoryStruct *data, char *coin)
{
    JsonParser *json_parser = json_parse_new_parser();
    GError *err = NULL;
    // Load data
    json_load_data(json_parser, data, JSON_DATA_TYPE_MEMORY, err);
    // Parse data
    JsonNode *json_root = json_parser_get_root(json_parser);
    JsonObject *root_obj = json_node_get_object(json_root);
    JsonArray *data_array = json_object_get_array_member(root_obj, "Data");
    // Setup data structures
    size_t arr_size = (size_t)json_array_get_length(data_array);
    CoinNewsData **news_arr = malloc(sizeof(CoinNewsData *) * arr_size);
    CoinNews *news_s = malloc(sizeof(CoinNews));
    news_s->news_arr = news_arr;
    news_s->size = arr_size;
    eina_lock_take(&ad->coin_news_mutex);
    bundle_add_byte(ad->coin_news, coin, news_s, sizeof(*news_s));
    // Load data from array members
    json_array_foreach_element(data_array, api_cryptocompare_news_parse_json_array_cb, news_arr);
    eina_lock_release(&ad->coin_news_mutex);
    // Cleanup
    g_error_free(err);
    g_object_unref(json_parser);
}

// Get news data for a coin from a thread
static void api_cryptocompare_news_thread_cb(void *data, Ecore_Thread *thread)
{
    char *coin = (char *)data;
    appdata_s *ad = get_appdata(NULL);
    // Check for existing news data
    CoinNewsData *news_s;
    size_t size;
    eina_lock_take(&ad->coin_news_mutex);
    int ret = bundle_get_byte(ad->coin_news, coin, (void **)&news_s, &size);
    eina_lock_release(&ad->coin_news_mutex);
    // Skip downloading data if already exists
    if (ret == BUNDLE_ERROR_NONE)
        return;
    // Api url
    char *url = new_string_printf(API_CRYPTOCOMPARE_NEWS, coin);
    // Send api request
    MemoryStruct *chunk = web_request_write_memory(url, NULL, NULL, REQUEST_TYPE_GET);
    // Cleanup
    free(url);
    // Request success
    if (chunk != NULL)
    {
        // Parse response
        api_cryptocompare_news_parse_json(ad, chunk, coin);
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

// Get news data for a coin
void api_cryptocompare_news(appdata_s *ad, char *coin, void *finished_cb, void *cancelled_cb)
{
    ecore_thread_run(api_cryptocompare_news_thread_cb, finished_cb, cancelled_cb, coin);
}

static void api_cryptocompare_full_obj_cb(JsonObject *object, const gchar *member_name, JsonNode *member_node, gpointer user_data)
{
    appdata_s *ad = get_appdata(NULL);
    bundle *coin_data = user_data;
    JsonObject *obj = json_node_get_object(member_node);
    JsonObject *data = json_object_get_object_member(obj, ad->currency);
    // Add price to global bundle for later use to reduce api calls
    CoinPriceData *price_data;
    size_t size;
    eina_lock_take(&ad->coin_price_data_mutex);
    int ret = bundle_get_byte(ad->coin_price_data, member_name, (void **)&price_data, &size);
    // Create new struct if doesn't exist
    if (ret != BUNDLE_ERROR_NONE)
        price_data = malloc(sizeof(CoinPriceData));
    // Update data
    price_data->price = json_object_get_double_member(data, "PRICE");
    price_data->low = json_object_get_double_member(data, "LOWDAY");
    price_data->high = json_object_get_double_member(data, "HIGHDAY");
    price_data->open = json_object_get_double_member(data, "OPEN24HOUR");
    price_data->volume = json_object_get_double_member(data, "VOLUME24HOURTO");
    price_data->percent_change = json_object_get_double_member(data, "CHANGEPCT24HOUR");
    price_data->absolute_change = json_object_get_double_member(data, "CHANGE24HOUR");
    // Add to bundle if doesn't exist
    if (ret != BUNDLE_ERROR_NONE)
        bundle_add_byte(ad->coin_price_data, member_name, price_data, sizeof(*price_data));
    // Update local bundle balance
    CoinData *bal_data = NULL;
    bundle_get_byte(coin_data, member_name, (void **)&bal_data, &size);
    bal_data->d_balance = bal_data->c_balance * price_data->price;
    eina_lock_release(&ad->coin_price_data_mutex);
    // Update icon url
    eina_lock_take(&ad->icon_urls_mutex);
    char *str;
    const char *img_url = json_object_get_string_member(data, "IMAGEURL");
    ret = bundle_get_str(ad->icon_urls, member_name, &str);
    if (ret != BUNDLE_ERROR_NONE)
        bundle_add_str(ad->icon_urls, member_name, img_url);
    eina_lock_release(&ad->icon_urls_mutex);
}

static void api_cryptocompare_full_parse_json(appdata_s *ad, MemoryStruct *data, bundle *coin_data)
{
    JsonParser *json_parser = json_parse_new_parser();
    GError *err = NULL;
    // Load data
    json_load_data(json_parser, data, JSON_DATA_TYPE_MEMORY, err);
    // Parse data
    JsonNode *json_root = json_parser_get_root(json_parser);
    JsonObject *root_obj = json_node_get_object(json_root);
    JsonObject *raw = json_object_get_object_member(root_obj, "RAW");
    json_object_foreach_member(raw, api_cryptocompare_full_obj_cb, coin_data);
    // Cleanup
    g_error_free(err);
    g_object_unref(json_parser);
}

// Get full data for a coin from a thread
static void api_cryptocompare_full_thread_cb(void *data, Ecore_Thread *thread)
{
    appdata_s *ad = get_appdata(NULL);
    struct ApiFullData *cb_data = data;
    // Generate url
    char *url = new_string_printf(API_CRYPTOCOMPARE_FULL, cb_data->from, ad->currency);
    // Send api request
    MemoryStruct *chunk = web_request_write_memory(url, NULL, NULL, REQUEST_TYPE_GET);
    // Request success
    if (chunk != NULL)
    {
        // Parse response
        api_cryptocompare_full_parse_json(ad, chunk, cb_data->coin_data);
        // Cleanup
        web_request_cleanup(chunk);
    }
    // Cleanup
    free(url);
    free(cb_data);
}

// Get full data for a coin async through another thread
void api_cryptocompare_full_async(char *from, bundle *coin_data, void *finished_cb)
{
    struct ApiFullData *cb_data = malloc(sizeof(struct ApiFullData));
    cb_data->from = from;
    cb_data->coin_data = coin_data;
    ecore_thread_run(api_cryptocompare_full_thread_cb, finished_cb, NULL, cb_data);
}

// Get full data for a coin synchronously in the same thread
void api_cryptocompare_full_sync(char *from, bundle *coin_data)
{
    struct ApiFullData *cb_data = malloc(sizeof(struct ApiFullData));
    cb_data->from = from;
    cb_data->coin_data = coin_data;
    api_cryptocompare_full_thread_cb(cb_data, NULL);
}

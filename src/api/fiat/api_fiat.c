#include <app_resource_manager.h>
#include <json-glib/json-glib.h>
#include <bundle.h>
#include "main.h"
#include "../../rw/json.h"
#include "api_fiat.h"

// Add new FiatData struct to bundle
static void api_fiat_parse_json_dump_cb(JsonObject *object, const gchar *member_name, JsonNode *member_node, gpointer user_data)
{
    bundle *b = user_data;
    // Create new FiatData struct to store data
    FiatData *fiat_data = malloc(sizeof(FiatData));
    // Json object with fiat data stored
    JsonObject *obj = json_node_get_object(member_node);
    // Load data into struct
    fiat_data->name = new_string_printf(json_object_get_string_member(obj, "name"));
    fiat_data->symbol = new_string_printf(json_object_get_string_member(obj, "symbol"));
    // Add struct pointer to bundle
    bundle_add_byte(b, member_name, fiat_data, sizeof(FiatData));
}

// Parse fiat currency data json
static void api_fiat_parse_json(appdata_s *ad, char *file_path)
{
    JsonParser *json_parser = json_parse_new_parser();
    JsonNode *json_root = NULL;
    JsonObject *root_obj = NULL;
    GError *err = NULL;
    // Load data
    json_load_data(json_parser, file_path, JSON_DATA_TYPE_FILE, err);
    // Parse data
    json_root = json_parser_get_root(json_parser);
    root_obj = json_node_get_object(json_root);
    // Add data for each fiat currency to bundle
    json_object_foreach_member(root_obj, api_fiat_parse_json_dump_cb, ad->fiat_data);
}

// Get fiat currency data
void api_fiat(appdata_s *ad)
{
    // Get json path
    char *file_path = NULL;
    app_resource_manager_get(APP_RESOURCE_TYPE_BIN, "data/currency_data.json", &file_path);
    // Parse json from file
    eina_lock_take(&ad->fiat_data_mutex);
    api_fiat_parse_json(ad, file_path);
    eina_lock_release(&ad->fiat_data_mutex);
    // Cleanup
    free(file_path);
}

// Get fiat currency data from a thread
void api_fiat_thread_cb(void *data, Ecore_Thread *thread)
{
    appdata_s *ad = (appdata_s *)data;
    api_fiat(ad);
}

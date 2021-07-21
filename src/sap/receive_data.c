#include <json-glib/json-glib.h>
#include "main.h"
#include "../rw/json.h"
#include "../rw/web.h"
#include "../rw/key.h"
#include "../ui/widgets/popup_view.h"
#include "receive_data.h"

static void parse_keys_data_obj_cb(JsonObject *object, const gchar *member_name, JsonNode *member_node, gpointer user_data)
{
    char *data = (char *)json_node_get_string(member_node);
    // Save key to key store if non-empty
    if (strcmp(data, ""))
        save_secure_data_overwrite((char *)member_name, data, strlen(data) + 1);
    // Remove key if data empty
    else
        remove_secure_data((char *)member_name);
}

void parse_keys_data(char *data, size_t size)
{
    appdata_s *ad = get_appdata(NULL);
    JsonParser *json_parser = json_parse_new_parser();
    GError *err = NULL;
    // Create new memory chunk
    MemoryStruct *mem = malloc(sizeof(MemoryStruct));
    mem->memory = data;
    mem->size = size;
    // Load data
    json_load_data(json_parser, mem, JSON_DATA_TYPE_MEMORY, err);
    // Parse data
    JsonNode *json_root = json_parser_get_root(json_parser);
    JsonObject *root_obj = json_node_get_object(json_root);
    json_object_foreach_member(root_obj, parse_keys_data_obj_cb, NULL);
    // Cleanup
    web_request_cleanup(mem);
    g_error_free(err);
    g_object_unref(json_parser);
    // Show success screen
    show_popup_view_ok(ad, "Keys Synced", "Keys have been successfully synced with device", elm_naviframe_item_pop, ad->naviframe);
}

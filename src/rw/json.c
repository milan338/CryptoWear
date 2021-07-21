#include <json-glib/json-glib.h>
#include "web.h"
#include "json.h"

// Create a new json parser
// Returned value must be freed with g_object_unref()
JsonParser *json_parse_new_parser()
{
    JsonParser *json_parser = NULL;
#if !GLIB_CHECK_VERSION(2, 35, 0)
    g_type_init();
#endif
    json_parser = json_parser_new();
    return json_parser;
}

// Load data from a source into a JsonParser
void json_load_data(JsonParser *json_parser, void *data, int data_type, GError *err)
{
    // Data storage
    MemoryStruct *chunk = NULL;
    char *file_path = NULL;
    switch (data_type)
    {
    case JSON_DATA_TYPE_MEMORY:
        chunk = data;
        json_parser_load_from_data(json_parser, (const gchar *)chunk->memory, (gsize)chunk->size, &err);
        break;
    case JSON_DATA_TYPE_FILE:
        file_path = data;
        json_parser_load_from_file(json_parser, file_path, &err);
        break;
    }
}

#ifndef JSON_H
#define JSON_H

#include <json-glib/json-glib.h>
#include "web.h"

#define JSON_DATA_TYPE_MEMORY 0
#define JSON_DATA_TYPE_FILE 1

JsonParser *json_parse_new_parser();
void json_load_data(JsonParser *json_parser, void *data, int data_type, GError *err);

#endif // JSON_H

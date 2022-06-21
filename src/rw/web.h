#ifndef WEB_H
#define WEB_H

#include <curl/curl.h>
#include "main.h"

#define MAX_RETRIES_MEM 5
#define WAIT_RETRY_MEM_S 1
#define MAX_RETRIES_FILE 10
#define WAIT_RETRY_FILE_US 100000

typedef struct MemoryStruct
{
    char *memory;
    size_t size;
} MemoryStruct;

typedef enum request_type_e
{
    REQUEST_TYPE_NONE,
    REQUEST_TYPE_GET,
    REQUEST_TYPE_POST
} request_type_e;

int web_request_write_file(char *url, char *post_data, struct curl_slist *http_headers, char *file_path, request_type_e request_type);
MemoryStruct *web_request_write_memory(char *url, char *post_data, struct curl_slist *http_headers, request_type_e request_type);
void web_request_cleanup(MemoryStruct *chunk);

#endif // WEB_H

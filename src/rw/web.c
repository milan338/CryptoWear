#include "web.h"
#include "../net/network.h"
#include "file.h"
#include "json.h"
#include <curl/curl.h>
#include <net_connection.h>

static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    // Out of memory
    if (ptr == NULL)
        return 0;
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

// Set common curl_easy_setopt options
static void web_request_curl_easy_setopts(CURL *curl_handler, char *url, char *post_data, struct curl_slist *http_headers, request_type_e request_type)
{
    appdata_s *ad = get_appdata(NULL);
    switch (request_type)
    {
    case REQUEST_TYPE_NONE:
        break;
    case REQUEST_TYPE_GET:
        curl_easy_setopt(curl_handler, CURLOPT_HTTPGET, 1);
        break;
    case REQUEST_TYPE_POST:
        curl_easy_setopt(curl_handler, CURLOPT_POST, 1);
        break;
    }
    curl_easy_setopt(curl_handler, CURLOPT_URL, url);
    curl_easy_setopt(curl_handler, CURLOPT_USERAGENT, "cryptowear-client");
    curl_easy_setopt(curl_handler, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handler, CURLOPT_FAILONERROR, 1L);
    if (post_data != NULL)
        curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDS, post_data);
    if (http_headers != NULL)
        curl_easy_setopt(curl_handler, CURLOPT_HTTPHEADER, http_headers);
    if (ad->proxy_addr != NULL)
        curl_easy_setopt(curl_handler, CURLOPT_PROXY, ad->proxy_addr);
}

// Send an http request and save response in memory, must be cleaned up afterwards using web_request_cleanup
MemoryStruct *web_request_write_memory(char *url, char *post_data, struct curl_slist *http_headers, request_type_e request_type)
{
    MemoryStruct *chunk = NULL;
    CURL *curl_handler = NULL;
    curl_handler = curl_easy_init();
    // No errors
    if (curl_handler)
    {
        int conn_err = create_new_connection();
        if (conn_err == CONNECTION_ERROR_NONE)
        {
            // Chunk memory be grown as needed by realloc
            chunk = malloc(sizeof(MemoryStruct));
            chunk->memory = malloc(1);
            chunk->size = 0;
            // Init curl handler
            web_request_curl_easy_setopts(curl_handler, url, post_data, http_headers, request_type);
            curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, write_memory_callback);
            curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, (void *)chunk);
            int n_retries = 0;
            while (n_retries <= MAX_RETRIES_MEM)
            {
                free(chunk->memory);
                free(chunk);
                chunk = malloc(sizeof(MemoryStruct));
                chunk->memory = malloc(1);
                chunk->size = 0;
                // Send request
                CURLcode curl_err = curl_easy_perform(curl_handler);
                if (curl_err != CURLE_OK)
                {
                    // Not retrying anymore
                    if (n_retries == MAX_RETRIES_MEM)
                        break;
                    sleep(WAIT_RETRY_MEM_S);
                    n_retries++;
                }
                else
                    break;
            }
        }
        destroy_existing_connection();
    }
    curl_easy_cleanup(curl_handler);
    return chunk;
}

// Send an http request and save response to file
int web_request_write_file(char *url, char *post_data, struct curl_slist *http_headers, char *file_path, request_type_e request_type)
{
    CURLcode curl_err = CURLE_OK;
    CURL *curl_handler = NULL;
    curl_handler = curl_easy_init();
    // No errors
    if (curl_handler)
    {
        int conn_err = create_new_connection();
        if (conn_err == CONNECTION_ERROR_NONE)
        {
            // Init file for writing
            FILE *fp = fopen(file_path, "wb");
            // Init curl handler
            web_request_curl_easy_setopts(curl_handler, url, post_data, http_headers, request_type);
            curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, fp);
            int n_retries = 0;
            while (n_retries <= MAX_RETRIES_FILE)
            {
                // Send request
                curl_err = curl_easy_perform(curl_handler);
                if (curl_err != CURLE_OK)
                {
                    // Not retrying anymore - cleanup
                    if (n_retries == MAX_RETRIES_FILE)
                    {
                        remove(file_path);
                        break;
                    }
                    usleep(WAIT_RETRY_FILE_US);
                    freopen(NULL, "wb", fp);
                    n_retries++;
                }
                else
                    break;
            }
            // Finish file operations
            fclose(fp);
        }
        destroy_existing_connection();
    }
    curl_easy_cleanup(curl_handler);
    return curl_err;
}

// Free resources from a web request
// Only necessary if request wrote response to memory
void web_request_cleanup(MemoryStruct *chunk)
{
    free(chunk->memory);
    free(chunk);
}

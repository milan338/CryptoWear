#include <app_resource_manager.h>
#include <bundle.h>
#include <curl/curl.h>
#include <net_connection.h>
#include <json-glib/json-glib.h>
#include <yaca/yaca_error.h>
#include <yaca/yaca_crypto.h>
#include "main.h"
#include "../rw/web.h"
#include "../rw/file.h"
#include "../rw/json.h"
#include "cryptocompare/api_cryptocompare.h"
#include "api_base.h"

// Download icon for crypto from a thread
static void download_crypto_icon_thread_cb(void *data, Ecore_Thread *thread)
{
    appdata_s *ad = get_appdata(NULL);
    char *crypto = (char *)data;
    char *file_name = new_string_printf("%s.png", crypto);
    char *icon_path = appdata_get_shared_filepath(file_name);
    char *img_url = NULL;
    lock_take(&ad->icon_urls_mutex);
    bundle_get_str(ad->icon_urls, crypto, &img_url);
    char *url = new_string_printf("https://cryptocompare.com%s", img_url);
    lock_release(&ad->icon_urls_mutex);
    // File exists online
    int curl_err;
    if (img_url[0] == '/')
        curl_err = web_request_write_file(url, NULL, NULL, icon_path, REQUEST_TYPE_NONE);
    // File exists in local appdata
    else
    {
        char *actual_path = NULL;
        app_resource_manager_get(APP_RESOURCE_TYPE_IMAGE, img_url, &actual_path);
        ecore_file_init();
        ecore_file_symlink(actual_path, icon_path);
        ecore_file_shutdown();
        curl_err = CURLE_OK;
        // Cleanup
        free(actual_path);
    }
    // Update file status bundle
    gint *status = NULL;
    size_t status_size;
    lock_take(&ad->file_status_mutex);
    bundle_get_byte(ad->file_status, icon_path, (void **)&status, &status_size);
    *status = curl_err == CURLE_OK ? 1 : 0;
    lock_release(&ad->file_status_mutex);
    // Cleanup
    free(file_name);
    free(icon_path);
    free(url);
}

void download_crypto_icon(char *crypto)
{
    ecore_file_init();
    char *file_name = new_string_printf("%s.png", crypto);
    char *icon_path = appdata_get_shared_filepath(file_name);
    // Download icon only if not already cached
    Eina_Bool exists = ecore_file_exists(icon_path);
    // Update file status
    update_file_status_bundle(icon_path, exists == EINA_FALSE ? 0 : 1);
    // Cleanup
    free(file_name);
    free(icon_path);
    ecore_file_shutdown();
    // Download icon in new thread if not already cached
    if (exists == EINA_FALSE)
        ecore_thread_run(download_crypto_icon_thread_cb, NULL, NULL, crypto);
}

// Compute the HMAC hash for a string
// Returned value must be freed with g_free()
char *sign_hmac_string(const char *message, const char *key, GChecksumType digest_type)
{
    gchar *encrypted_msg = g_compute_hmac_for_string(digest_type,
                                                     (const guchar *)key,
                                                     (gsize)strlen(key),
                                                     (const gchar *)message,
                                                     (gssize)strlen(message));
    return (char *)encrypted_msg;
}

// Generate a random string
// Returned value must be freed
char *random_str(int len)
{
    size_t size = sizeof(char) * len;
    char *rand_str = malloc(size);
    yaca_error_e err = yaca_randomize_bytes(rand_str, size);
    if (err != YACA_ERROR_NONE)
    {
        if (rand_str != NULL)
            free(rand_str);
        return NULL;
    }
    else
        return rand_str;
}

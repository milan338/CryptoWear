#include <bundle.h>
#include <json-glib/json-glib.h>
#include "main.h"
#include "file.h"

// Get a filepath from the application shared data directory
// Returned filepath must be freed
char *appdata_get_shared_filepath(char *file_name)
{
    char *resource_path = app_get_shared_data_path();
    char *file_path = new_string_printf("%s%s", resource_path, file_name);
    return file_path;
}

// Update file status in bundle - 0 for unavailable, 1 for available
void update_file_status_bundle(char *path, int status)
{
    appdata_s *ad = get_appdata(NULL);
    // Get flag
    gint *flag = NULL;
    size_t flag_size;
    lock_take(&ad->file_status_mutex);
    int ret = bundle_get_byte(ad->file_status, path, (void **)&flag, &flag_size);
    // Create new bundle entry if non-existent
    if (ret != BUNDLE_ERROR_NONE)
    {
        flag = malloc(sizeof(gint));
        *flag = status;
        bundle_add_byte(ad->file_status, path, flag, sizeof(*flag));
    }
    else
        *flag = status;
    lock_release(&ad->file_status_mutex);
}

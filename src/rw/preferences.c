#include <app_preference.h>
#include "main.h"
#include "preferences.h"

// Called whenever a preference is changed
static void preference_updated_cb(const char *key, void *user_data)
{
    // Pointer to appdata to change
    char *to_var = user_data;
    bool ret;
    char *string_output;
    // Get current value of preference
    ret = get_preference_char(key, &string_output);
    // Update appdata string
    strcpy(to_var, string_output);
    // Cleanup
    free_preference(string_output);
}

void init_preferences(appdata_s *ad)
{
    bool ret;
    char *string_output;
    void (*cb)(const char *key, void *cb_data);
    PreferenceData preference_data[] = {
        {"currency", "USD", ad->currency, preference_updated_cb, ad->currency},
        {"data provider", "Binance", ad->data_provider, preference_updated_cb, ad->data_provider}};
    int preference_data_size = sizeof(preference_data) / sizeof(preference_data[0]);
    // String preferences
    for (int i = 0; i < preference_data_size; i++)
    {
        // Check if data exists
        preference_is_existing(preference_data[i].key, &ret);
        // Init data if it doesn't exist
        if (!ret)
        {
            ret = set_preference_char(preference_data[i].key,
                                      preference_data[i].default_val);
            cb = preference_data[i].cb;
            cb(preference_data[i].key, preference_data[i].to_var);
        }
        // Get data
        else
        {
            ret = get_preference_char(preference_data[i].key, &string_output);
            strcpy(preference_data[i].to_var, string_output);
            free_preference(string_output);
        }
        // Set preference update callback
        preference_set_changed_cb(preference_data[i].key,
                                  preference_data[i].cb,
                                  preference_data[i].cb_data);
    }
}

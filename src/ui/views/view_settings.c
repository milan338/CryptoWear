#include <app_preference.h>
#include <bundle.h>
#include "main.h"
#include "../../rw/preferences.h"
#include "../../api/api_data.h"
#include "../../api/fiat/api_fiat.h"
#include "../../sap/accessory.h"
#include "../widgets/list_view.h"
#include "../widgets/popup_view.h"
#include "view_settings.h"

// Pointers to genlist items used to update their displayed current setting
static Elm_Widget_Item *currency_it;
static Elm_Widget_Item *provider_it;
static Elm_Widget_Item **items[] = {&currency_it, &provider_it};
static int items_num = (sizeof(items) / sizeof(items[0]));

// Update subtext for genlist item when its associated genlist is popped from the naviframe stack
static Eina_Bool update_subtext_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Widget_Item *it = data;
    elm_genlist_item_fields_update(it, "elm.text.1", ELM_GENLIST_ITEM_FIELD_TEXT);
    return EINA_TRUE;
}

static void cache_clear_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = get_appdata(NULL);
    bool clear = data;
    if (clear)
    {
        // Remove data dir and its contents
        ecore_file_init();
        ecore_file_recursive_rm(app_get_shared_data_path());
        ecore_file_shutdown();
        // Clear exchange data
        exchange_data_clear(false);
    }
    // Go to previous screen
    elm_naviframe_item_pop(ad->naviframe);
}

static void cache_clear(appdata_s *ad)
{
    // Show popup confirmation
    show_popup_view_yesno(ad,
                          "Clear Cache",
                          "Application cache will be cleared."
                          " Resources such as icons will need to be re-downloaded."
                          " Continue?",
                          cache_clear_cb,
                          (void *)true,
                          (void *)false);
}

static void settings_clear_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = get_appdata(NULL);
    bool clear = data;
    if (clear)
    {
        // Clear all user preferences
        preference_remove_all();
        // Re-init preferences
        init_preferences(ad);
        // Fetch new data on next request
        exchange_data_clear(false);
    }
    // Go to previous screen
    elm_naviframe_item_pop(ad->naviframe);
    // Refresh settings screen
    for (int i = 0; i < items_num; i++)
        elm_genlist_item_fields_update(*items[i], "elm.text.1", ELM_GENLIST_ITEM_FIELD_TEXT);
}

static void settings_clear(appdata_s *ad)
{
    // Show popup confirmation
    show_popup_view_yesno(ad,
                          "Reset Settings",
                          "Your settings will be reset to application defaults. Continue?",
                          settings_clear_cb,
                          (void *)true,
                          (void *)false);
}

static void set_preference(char *preference, char *value)
{
    appdata_s *ad = get_appdata(NULL);
    // Update selected currency
    bool ret;
    ret = set_preference_char(preference, value);
    // Update radio selections
    elm_radio_value_set(ad->radio_group, eina_hash_superfast(value, strlen(value)));
    // Reset radio group
    evas_object_del(ad->radio_group);
    ad->radio_group = NULL;
    // Go to previous screen
    elm_naviframe_item_pop(ad->naviframe);
}

static void set_provider(char *provider)
{
    set_preference("data provider", provider);
}

static void view_settings_provider(appdata_s *ad)
{
    // Create root radio button to act as persistent radio group
    ad->radio_group = elm_radio_add(ad->naviframe);
    // Update current currency for radio button
    int goto_hash = eina_hash_superfast(ad->data_provider, strlen(ad->data_provider));
    elm_radio_value_set(ad->radio_group, goto_hash);
    // Push genlist to naviframe stack
    Elm_Object_Item *nf_it = show_list_view(ad);
    // Add genlist items
    genlist_title_add(ad, "Data Provider");
    for (int i = 0; i < DATA_PROVIDER_NUM; i++)
        genlist_radio_add(ad,
                          data_providers_list[i],
                          goto_hash,
                          set_provider,
                          (void *)data_providers_list[i]);
    genlist_padding_add(ad);
    // Refresh settings page
    elm_naviframe_item_pop_cb_set(nf_it, (Elm_Naviframe_Item_Pop_Cb)update_subtext_pop_cb, provider_it);
}

static void set_currency(char *currency)
{
    set_preference("currency", currency);
    // Clear exchange data - fetch data in new currency on next request
    exchange_data_clear(false);
}

static void view_settings_currency(appdata_s *ad)
{
    // Create root radio button to act as persistent radio group
    ad->radio_group = elm_radio_add(ad->naviframe);
    // Update current currency for radio button
    int goto_hash = eina_hash_superfast(ad->currency, strlen(ad->currency));
    elm_radio_value_set(ad->radio_group, goto_hash);
    // Push genlist to naviframe stack
    Elm_Object_Item *nf_it = show_list_view(ad);
    // Add genlist items
    genlist_title_add(ad, "Currency");
    for (int i = 0; i < FIAT_ARRAY_SIZE; i++)
    {
        genlist_fiat_add(ad,
                         fiat_array[i],
                         goto_hash,
                         set_currency,
                         (void *)fiat_array[i]);
    }
    genlist_padding_add(ad);
    // Refresh settings page
    elm_naviframe_item_pop_cb_set(nf_it, (Elm_Naviframe_Item_Pop_Cb)update_subtext_pop_cb, currency_it);
}

static void view_settings_get_keys_connected_cb()
{
    appdata_s *ad = get_appdata(NULL);
    // Begin handshake
    int ret = companion_send_secure_data("{\"requestType\":\"getKeys\"}");
    if (!ret)
        show_popup_view_ok(ad, "Failed to Sync", "Failed to send data to device", elm_naviframe_item_pop, ad->naviframe);
}

static void view_settings_get_keys(appdata_s *ad)
{
    // Find available SAP peers and connect
    find_peer_agent(view_settings_get_keys_connected_cb, NULL);
}

void view_settings(appdata_s *ad)
{
    // Push genlist to naviframe stack
    show_list_view(ad);
    // Add genlist items
    genlist_title_add(ad, "Settings");
    currency_it = genlist_setting_add(ad, "Currency", view_settings_currency, ad);
    provider_it = genlist_setting_add(ad, "Data Provider", view_settings_provider, ad);
    genlist_line_add(ad, "Clear Cache", cache_clear, ad);
    genlist_line_add(ad, "Sync Keys", view_settings_get_keys, ad);
    genlist_line_add(ad, "Reset Settings", settings_clear, ad);
    genlist_padding_add(ad);
}

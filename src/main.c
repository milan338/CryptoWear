#include <app_resource_manager.h>
#include <bundle.h>
#include <curl/curl.h>
#include <EWebKit.h>
#include "main.h"
#include "rw/preferences.h"
#include "rw/web.h"
#include "sap/accessory.h"
#include "api/api_data.h"
#include "api/fiat/api_fiat.h"
#include "api/coingecko/api_coingecko.h"
#include "ui/widgets/list_view.h"
#include "ui/views/view_home.h"

static void win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
    ui_app_exit();
}

// Back button pressed callback
static void win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = data;
    // Exit if popping not enabled
    if (!ad->enable_pop)
        return;
    elm_naviframe_item_pop(ad->naviframe);
    Elm_Object_Item *bottom_it = elm_naviframe_bottom_item_get(ad->naviframe);
    // Naviframe stack empty - exiting app
    if (bottom_it == NULL)
    {
        ad->win_lower = true;
        // Put window in hidden state
        elm_win_lower(ad->win);
    }
}

appdata_s *get_appdata(appdata_s *ad)
{
    static appdata_s *appdata;
    if (ad == NULL)
        return appdata;
    else
    {
        appdata = ad;
        return NULL;
    }
}
static void create_base_gui(appdata_s *ad)
{
    // Set static appdata pointer
    get_appdata(ad);
    // Create main window
    ad->win_lower = false;
    ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
    // Set autodelete policy
    elm_win_autodel_set(ad->win, EINA_TRUE);
    // Set rotation policy
    if (elm_win_wm_rotation_supported_get(ad->win))
    {
        int rots[4] = {0, 90, 180, 270};
        elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
    }
    // Set main window callbacks
    evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
    eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);
    // Create conformant
    ad->conform = elm_conformant_add(ad->win);
    elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
    elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
    evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(ad->win, ad->conform);
    evas_object_show(ad->conform);
    // Create naviframe
    ad->naviframe = elm_naviframe_add(ad->conform);
    elm_object_content_set(ad->conform, ad->naviframe);
    // Create circle surface
    ad->circle_suface = eext_circle_surface_naviframe_add(ad->naviframe);
    // Show window after base gui is set up
    evas_object_show(ad->win);
    // Get screen data
    evas_object_geometry_get(ad->naviframe, NULL, NULL, &ad->width, &ad->height);
    // Setup item classes for genlist items
    create_genlist_item_classes(ad);
    // Show main menu
    view_home(ad);
}

// Hook before main event loop starts
static bool app_create(void *data)
{
    appdata_s *ad = data;
    // Enable naviframe popping
    ad->enable_pop = true;
    // Load user preferences
    init_preferences(ad);
    // Init app resource manager
    app_resource_manager_init();
    // Init curl
    curl_global_init(CURL_GLOBAL_ALL);
    // Init eina modules
    eina_init();
    // Init SAP
    sap_init(ad);
    // Create new mutexes
    eina_lock_new(&ad->file_status_mutex);
    eina_lock_new(&ad->coin_list_mutex);
    eina_lock_new(&ad->fiat_data_mutex);
    eina_lock_new(&ad->coin_price_data_mutex);
    eina_lock_new(&ad->coin_news_mutex);
    eina_lock_new(&ad->icon_urls_mutex);
    // Create bundles
    ad->exchange_data = bundle_create();
    ad->fiat_data = bundle_create();
    ad->file_status = bundle_create();
    ad->coin_price_data = bundle_create();
    ad->coin_news = bundle_create();
    ad->icon_urls = bundle_create();
    // Get crypto full names
    ad->crypto_names = NULL;
    ecore_thread_run(api_coingecko_coin_list_thread_cb, NULL, NULL, ad);
    // Get fiat data
    ecore_thread_run(api_fiat_thread_cb, NULL, NULL, ad);
    // Substitute local icons for online icons
    for (int i = 0; i < local_icons_size; i++)
        bundle_add_str(ad->icon_urls, local_icons[i].coin, local_icons[i].path);
    // Update last updated time
    ad->last_updated = ecore_time_unix_get();
    // Init UI
    create_base_gui(ad);
    // Start main application loop - false to terminate app
    return true;
}

// Handle launch request
static void app_control(app_control_h app_control, void *data)
{
    appdata_s *ad = data;
    // Resume app from hidden state
    if (ad->win_lower)
    {
        ad->win_lower = false;
        view_home(ad);
    }
}

// Actions when application becomes invisible
static void app_pause(void *data)
{
}

// Actions when application becomes visible
static void app_resume(void *data)
{
}

// Release all resources
static void app_terminate(void *data)
{
    appdata_s *ad = data;
    // Clear naviframe stack
    Elm_Object_Item *bottom_it = elm_naviframe_bottom_item_get(ad->naviframe);
    if (bottom_it != NULL)
        elm_naviframe_item_pop_to(bottom_it);
    // Release bundle / dynamic array resources
    bundle_free(ad->crypto_names);
    bundle_foreach(ad->exchange_data, (bundle_iterator_t)exchange_data_bundle_cleanup_cb, NULL);
    bundle_free(ad->exchange_data);
    bundle_foreach(ad->fiat_data, (bundle_iterator_t)fiat_data_bundle_cleanup_cb, NULL);
    bundle_free(ad->fiat_data);
    bundle_foreach(ad->coin_news, (bundle_iterator_t)coin_news_bundle_cleanup_cb, NULL);
    bundle_free(ad->coin_news);
    bundle_free(ad->file_status);
    bundle_free(ad->coin_price_data);
    bundle_free(ad->icon_urls);
    // Release webview resources
    ewk_shutdown();
    // Release app resource manager resources
    app_resource_manager_release();
    // Cleanup curl
    curl_global_cleanup();
    // Cleanup mutexes
    eina_lock_free(&ad->file_status_mutex);
    eina_lock_free(&ad->coin_list_mutex);
    eina_lock_free(&ad->fiat_data_mutex);
    eina_lock_free(&ad->coin_price_data_mutex);
    eina_lock_free(&ad->coin_news_mutex);
    eina_lock_free(&ad->icon_urls_mutex);
    // Cleanup SAP
    sap_shutdown();
    // Shutdown eina modules
    eina_shutdown();
}

// APP_EVENT_LANGUAGE_CHANGED
static void ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
    char *locale = NULL;
    system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
    elm_language_set(locale);
    free(locale);
    return;
}

// APP_EVENT_DEVICE_ORIENTATION_CHANGED
static void ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
}

// APP_EVENT_REGION_FORMAT_CHANGED
static void ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
}

// APP_EVENT_LOW_BATTERY
static void ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
}

// APP_EVENT_LOW_MEMORY
static void ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
}

int main(int argc, char *argv[])
{
    appdata_s ad = {
        0,
    };
    int ret = 0;

    ui_app_lifecycle_callback_s event_callback = {
        0,
    };
    app_event_handler_h handlers[5] = {
        NULL,
    };

    event_callback.create = app_create;
    event_callback.terminate = app_terminate;
    event_callback.pause = app_pause;
    event_callback.resume = app_resume;
    event_callback.app_control = app_control;

    ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
    ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

    ret = ui_app_main(argc, argv, &event_callback, &ad);
    if (ret != APP_ERROR_NONE)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
    }

    return ret;
}

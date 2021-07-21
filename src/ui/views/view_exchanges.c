#include <bundle.h>
#include "main.h"
#include "../../api/api_data.h"
#include "../../rw/web.h"
#include "../../rw/key.h"
#include "../widgets/list_view.h"
#include "../widgets/progressbar_view.h"
#include "../widgets/popup_view.h"
#include "../../api/coinspot/api_coinspot.h"
#include "../../api/coinbase/api_coinbase.h"
#include "view_exchange_base.h"
#include "view_settings.h"
#include "view_exchanges.h"

static exchange_views_s exchange_views[] = {
    {"COINBASE", "Coinbase", "coinbase", "coinbase_key", "coinbase_secret", api_coinbase},
    {"COINSPOT", "CoinSpot", "coinspot", "coinspot_key", "coinspot_secret", api_coinspot}};
static int exchange_views_size = sizeof(exchange_views) / sizeof(exchange_views[0]);

static Eina_Bool view_exchange_thread_cancelled_popup_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = get_appdata(NULL);
    // Pop spinner from naviframe
    elm_naviframe_item_pop_to(ad->exchanges_it);
    return EINA_TRUE;
}

// Callback function after downloading from api
static void view_exchange_thread_finished_cb(void *data, Ecore_Thread *thread)
{
    appdata_s *ad = get_appdata(NULL);
    // Re-enable naviframe popping
    ad->enable_pop = true;
    // Show exchange menu
    exchange_views_s *ev = (exchange_views_s *)data;
    view_exchange_init(ad, ev->exchange, ev->label, ev->bundle_key);
}

// Callback function when api download fails
static void view_exchange_thread_cancelled_cb(void *data, Ecore_Thread *thread)
{
    appdata_s *ad = get_appdata(NULL);
    // Re-enable naviframe popping
    ad->enable_pop = true;
    // Show error message
    Elm_Object_Item *nf_it = show_popup_view_ok(ad, "API Call Failure", "Failed to execute API call", elm_naviframe_item_pop, ad->naviframe);
    // Pop past spinner on popup exit and show exchange menu only with key links
    elm_naviframe_item_pop_cb_set(nf_it, (Elm_Naviframe_Item_Pop_Cb)view_exchange_thread_cancelled_popup_pop_cb, NULL);
}

// Callback function to display exchange view
static void view_exchange(void *data)
{
    appdata_s *ad = get_appdata(NULL);
    // Check if data for selected exchange exists
    exchange_views_s *ev = data;
    ExchangeData *exchange_data = NULL;
    size_t exchange_size;
    int ret = bundle_get_byte(ad->exchange_data, ev->bundle_key, (void **)&exchange_data, &exchange_size);
    // Show progress spinner if data needs to be downloaded
    if (ret != BUNDLE_ERROR_NONE)
    {
        // Check if api keys exist
        char *key = (char *)get_secure_data((char *)ev->_key);
        char *secret = (char *)get_secure_data((char *)ev->_secret);
        // Keys exist
        if (key != NULL && secret != NULL)
        {
            // Cleanup
            free(key);
            free(secret);
            // Push loading spinner to view
            show_progress_spinner(ad);
            // Disable naviframe popping
            ad->enable_pop = false;
            // Download data and show view afterwards
            ecore_thread_run(ev->api_cb, view_exchange_thread_finished_cb, view_exchange_thread_cancelled_cb, ev);
        }
        // Keys don't exist
        else
        {
            show_popup_view_ok(ad, "API Keys Missing", "API key and / or secret are missing. Please sync them with CryptoWear Companion in settings", elm_naviframe_item_pop, ad->naviframe);
            if (key != NULL)
                free(key);
            if (secret != NULL)
                free(secret);
        }
    }
    // Skip progress spinner and push exchange view to naviframe
    else
        view_exchange_init(ad, ev->exchange, ev->label, ev->bundle_key);
}

void view_exchanges_all(appdata_s *ad)
{
    // Push genlist to naviframe stack
    Elm_Object_Item *nf_it = show_list_view(ad);
    ad->exchanges_it = nf_it;
    // Add genlist items
    genlist_title_add(ad, "Exchanges");
    for (int i = 0; i < exchange_views_size; i++)
        genlist_line_add(ad, exchange_views[i].label, view_exchange, (void *)&exchange_views[i]);
    genlist_padding_add(ad);
}

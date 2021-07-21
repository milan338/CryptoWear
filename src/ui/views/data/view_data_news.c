#include <bundle.h>
#include <EWebKit.h>
#include "main.h"
#include "../../widgets/list_view.h"
#include "../../widgets/progressbar_view.h"
#include "../../widgets/article_view.h"
#include "../../widgets/popup_view.h"
#include "../../../api/cryptocompare/api_cryptocompare.h"
#include "view_data_news.h"

// TODO refactor this out into a separate widget which can directly take the crypto label as an argument
// and then have this function just call that function, passing through the correct label
// also do this for all the other views like this one i.e. graph, news, etc.

static char *title = NULL;

static Eina_Bool view_data_news_failed_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = get_appdata(NULL);
    // Pop spinner from naviframe
    elm_naviframe_item_pop_to(ad->balances_it);
    return EINA_TRUE;
}

static Eina_Bool view_data_news_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = get_appdata(NULL);
    if (title != NULL)
        free(title);
    title = NULL;
    // Pop spinner from naviframe
    elm_naviframe_item_pop_to(ad->balances_it);
    return EINA_TRUE;
}

static void view_data_news_failed_cb(void *data, Ecore_Thread *thread)
{
    appdata_s *ad = get_appdata(NULL);
    // Re-enable naviframe popping
    ad->enable_pop = true;
    // Show error message
    Elm_Object_Item *nf_it = show_popup_view_ok(ad, "API Call Failure", "Failed to fetch news", elm_naviframe_item_pop, ad->naviframe);
    // Pop past spinner on popup exit and show exchange menu only with key links
    elm_naviframe_item_pop_cb_set(nf_it, (Elm_Naviframe_Item_Pop_Cb)view_data_news_failed_pop_cb, NULL);
}

static void view_data_news_loaded_cb(void *data, Ecore_Thread *thread)
{
    char *symbol = data;
    appdata_s *ad = get_appdata(NULL);
    // re-enable naviframe popping
    ad->enable_pop = true;
    // Get news data array
    CoinNews *news = NULL;
    size_t size;
    eina_lock_take(&ad->coin_news_mutex);
    bundle_get_byte(ad->coin_news, symbol, (void **)&news, &size);
    eina_lock_release(&ad->coin_news_mutex);
    // Get crypto name
    char *name = NULL;
    eina_lock_take(&ad->coin_list_mutex);
    int err = bundle_get_str(ad->crypto_names, symbol, &name);
    eina_lock_release(&ad->coin_list_mutex);
    if (err != BUNDLE_ERROR_NONE)
        title = new_string_printf("%s News", symbol);
    else
        title = new_string_printf("%s News", name);
    // Push genlist to naviframe stack
    Elm_Object_Item *nf_it = show_list_view(ad);
    // Expand naviframe text vertically instead of horizontally scrolling
    elm_genlist_mode_set(ad->genlist, ELM_LIST_COMPRESS);
    // Add genlist items
    genlist_title_add(ad, title);
    for (size_t i = 0; i < news->size; i++)
        genlist_news_add(ad, news->news_arr[i], show_article_view, news->news_arr[i]);
    // Cleanup on menu exit
    elm_naviframe_item_pop_cb_set(nf_it, (Elm_Naviframe_Item_Pop_Cb)view_data_news_pop_cb, NULL);
}

void view_data_news(int i)
{
    appdata_s *ad = get_appdata(NULL);
    char *symbol = ad->coin_data[i].symbol;
    // Push loading spinner to view
    show_progress_spinner(ad);
    // Disable naviframe popping
    ad->enable_pop = false;
    // Download news data if doesn't exist, proceed to next page
    api_cryptocompare_news(ad, symbol, view_data_news_loaded_cb, view_data_news_failed_cb);
}

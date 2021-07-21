#include <bundle.h>
#include "main.h"
#include "../widgets/list_view.h"
#include "../widgets/pie_chart_view.h"
#include "view_balances.h"
#include "../../api/api_data.h"
#include "../../rw/key.h"
#include "view_exchange_base.h"

static const PieChartColors chart_colors[] = {
    {RGBA_NORM(79, 179, 193, 255)},
    {RGBA_NORM(37, 137, 173, 255)},
    {RGBA_NORM(23, 93, 145, 255)},
    {RGBA_NORM(32, 51, 110, 255)},
    {RGBA_NORM(64, 64, 64, 255)}};

static Eina_Bool view_exchange_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = data;
    // Free resources
    free(ad->coin_data);
    ad->coin_data = NULL;
    ad->coin_data_size = 0;
    // Clear progress spinner if exists
    elm_naviframe_item_pop_to(ad->exchanges_it);
    // Clear current exchange
    free(ad->current_exchange);
    return EINA_TRUE;
}

static void view_exchange_overview(appdata_s *ad)
{
    show_pie_chart_view(ad, ad->coin_data, ad->coin_data_size, (PieChartColors *)&chart_colors, PIE_CHART_STYLE_DONUT);
}

void view_exchange_init(appdata_s *ad, const char *exchange, const char *title, const char *bundle_key)
{
    // Push genlist to naviframe stack
    Elm_Object_Item *nf_it = show_list_view(ad);
    // Add genlist items
    genlist_title_add(ad, title);
    // Update current exchange
    ad->current_exchange = new_string_printf(exchange);
    // Get exchange data
    ExchangeData *exchange_data = NULL;
    size_t exchange_size;
    bundle_get_byte(ad->exchange_data, bundle_key, (void **)&exchange_data, &exchange_size);
    // Create new temporary CoinData struct from Eina_Array
    ad->coin_data = malloc(sizeof(CoinData) * eina_array_count(exchange_data->coin_data_sorted));
    ad->coin_data_size = eina_array_count(exchange_data->coin_data_sorted);
    array_iter_index = 0;
    eina_array_foreach(exchange_data->coin_data_sorted, coin_data_copy_cb, ad->coin_data);
    // Add remaining UI links
    genlist_line_add(ad, "Overview", view_exchange_overview, ad);
    genlist_line_add(ad, "Balances", view_balances, ad);
    // Free temporary CoinData array on menu exit
    elm_naviframe_item_pop_cb_set(nf_it, (Elm_Naviframe_Item_Pop_Cb)view_exchange_pop_cb, ad);
    genlist_padding_add(ad);
}

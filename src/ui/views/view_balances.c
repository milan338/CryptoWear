#include "main.h"
#include "../widgets/list_view.h"
#include "data/view_data_main.h"
#include "view_balances.h"

static int genlist_refresh_items_cb(void *data)
{
    appdata_s *ad = data;
    int posret;
    // Get item at screen centre
    Elm_Widget_Item *it = elm_genlist_at_xy_item_get(ad->genlist, ad->width / 2, ad->height / 2, &posret);
    elm_genlist_realized_items_update(ad->genlist);
    // This stops the genlist from scrolling to the top on items refresh
    elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
    return ECORE_CALLBACK_CANCEL;
}

void view_balances(appdata_s *ad)
{
    // Push genlist to naviframe stack
    ad->balances_it = show_list_view(ad);
    // Add genlist items
    genlist_title_add(ad, "Balances");
    for (size_t i = 0; i < ad->coin_data_size; i++)
        genlist_crypto_add(ad, ad->coin_data[i].symbol, view_data_main, (void *)i);
    genlist_padding_add(ad);
    // Reload items in future once to compensate for downlaoding icons
    ecore_timer_add(2, genlist_refresh_items_cb, ad);
}

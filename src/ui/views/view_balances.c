#include "main.h"
#include "../widgets/list_view.h"
#include "data/view_data_main.h"
#include "view_balances.h"

static int n_refreshes;

static int genlist_refresh_items_cb(void *data)
{
    appdata_s *ad = data;
    int posret;
    int ret = n_refreshes++ < MAX_REFRESHES ? ECORE_CALLBACK_RENEW : ECORE_CALLBACK_CANCEL;
    // Get item at screen centre
    Elm_Widget_Item *centre_it = elm_genlist_at_xy_item_get(ad->genlist, ad->width / 2, ad->height / 2, &posret);
    if (centre_it == NULL)
        return ret;
    Elm_Widget_Item *prev_it = elm_genlist_item_prev_get(centre_it);
    Elm_Widget_Item *next_it = elm_genlist_item_next_get(centre_it);
    // Refresh item in screen centre, and item directly above and below
    elm_genlist_item_fields_update(centre_it, "elm.icon", ELM_GENLIST_ITEM_FIELD_CONTENT);
    if (prev_it != NULL)
        elm_genlist_item_fields_update(prev_it, "elm.icon", ELM_GENLIST_ITEM_FIELD_CONTENT);
    if (next_it != NULL)
        elm_genlist_item_fields_update(next_it, "elm.icon", ELM_GENLIST_ITEM_FIELD_CONTENT);
    return ret;
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
    n_refreshes = 0;
    ecore_timer_add(1, genlist_refresh_items_cb, ad);
}

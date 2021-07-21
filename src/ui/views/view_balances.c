#include "main.h"
#include "../widgets/list_view.h"
#include "data/view_data_main.h"
#include "view_balances.h"

void view_balances(appdata_s *ad)
{
    // Push genlist to naviframe stack
    ad->balances_it = show_list_view(ad);
    // Add genlist items
    genlist_title_add(ad, "Balances");
    for (size_t i = 0; i < ad->coin_data_size; i++)
        genlist_crypto_add(ad, ad->coin_data[i].symbol, view_data_main, (void *)i);
    genlist_padding_add(ad);
}

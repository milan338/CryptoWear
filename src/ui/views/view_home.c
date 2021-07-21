#include "main.h"
#include "../../api/api_data.h"
#include "../widgets/list_view.h"
#include "view_exchanges.h"
#include "view_settings.h"
#include "view_home.h"

void view_home(appdata_s *ad)
{
    // Push genlist to naviframe stack
    show_list_view(ad);
    // Add genlist items
    genlist_title_add(ad, "CryptoWear");
    genlist_line_add(ad, "Exchanges", view_exchanges_all, ad);
    genlist_line_add(ad, "Settings", view_settings, ad);
    // genlist_line_add(ad, "Refresh Data", exchange_data_clear, true);
    ad->refresh_it = genlist_refresh_add(ad, "Refresh Data", exchange_data_clear, (void *)true);
    genlist_padding_add(ad);
}

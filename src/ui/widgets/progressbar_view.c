#include "main.h"
#include "progressbar_view.h"

// Push empty progressbar spinner to naviframe
Elm_Object_Item *show_progress_spinner(appdata_s *ad)
{
    ad->progressbar = elm_progressbar_add(ad->naviframe);
    // Set progressbar style
    elm_object_style_set(ad->progressbar, "process/popup/small");
    elm_progressbar_value_set(ad->progressbar, 0.0);
    evas_object_size_hint_align_set(ad->progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(ad->progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    // Set 'pulse' animation mode
    elm_progressbar_pulse_set(ad->progressbar, EINA_TRUE);
    elm_progressbar_pulse(ad->progressbar, EINA_TRUE);
    evas_object_show(ad->progressbar);
    // Push progressbar to naviframe
    Elm_Object_Item *nf_it = NULL;
    nf_it = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->progressbar, "empty");
    return nf_it;
}

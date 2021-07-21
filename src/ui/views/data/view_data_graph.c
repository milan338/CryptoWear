#include <EWebKit.h>
#include "main.h"
#include "../../widgets/list_view.h"
#include "view_data.h"
#include "view_data_graph.h"

static int curr_i;
static int time_range_i;
static char *graph_html;
static const TimelineData timeline_data[] = {
    {"1 Day", "1D"},
    {"1 Month", "1M"},
    {"3 Months", "3M"},
    {"1 Year", "12M"},
    {"5 Years", "60M"},
    {"All", "ALL"}};

static Eina_Bool view_balance_data_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
    // Close webview instance
    ewk_shutdown();
    // Cleanup stored HTML string
    free(graph_html);
    return EINA_TRUE;
}

static void timeline_set_range(int i)
{
    appdata_s *ad = get_appdata(NULL);
    // Generate HTML
    int j = curr_i;
    // Update radio button data
    time_range_i = i;
    elm_radio_value_set(ad->radio_group,
                        eina_hash_superfast(timeline_data[time_range_i].name,
                                            strlen(timeline_data[time_range_i].name)));
    // Cleanup existing HTML string
    free(graph_html);
    graph_html = NULL;
    // Load new HTML into memory
    graph_html = new_string_printf(graph_html_template,
                                   ad->coin_data[j].symbol,
                                   ad->currency,
                                   ad->data_provider,
                                   ad->coin_data[j].symbol,
                                   ad->currency,
                                   (int)((double)ad->width * 0.9),
                                   (int)((double)ad->height * 0.9),
                                   timeline_data[i].data);
    // Update button text
    elm_object_text_set(ad->web_view_btn, timeline_data[i].name);
    // Go to previous view
    elm_naviframe_item_pop(ad->naviframe);
}

static Eina_Bool timeline_btn_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = data;
    // Load HTML string into webview - prevents webview disappearing sometimes
    ewk_view_html_string_load(ad->web_view, graph_html, NULL, NULL);
    // Reset radio group
    evas_object_del(ad->radio_group);
    ad->radio_group = NULL;
    return EINA_TRUE;
}

void timeline_more_option_closed_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = get_appdata(NULL);
    // Load HTML string into webview - prevents webview disappearing sometimes
    ewk_view_html_string_load(ad->web_view, graph_html, NULL, NULL);
}

static void timeline_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = get_appdata(NULL);
    // Create root radio button to act as persistent radio group
    ad->radio_group = elm_radio_add(ad->naviframe);
    // Update current currency for radio button
    int goto_hash = eina_hash_superfast(timeline_data[time_range_i].name,
                                        strlen(timeline_data[time_range_i].name));
    elm_radio_value_set(ad->radio_group, goto_hash);
    // Push genlist to naviframe stack
    Elm_Object_Item *nf_it = show_list_view(ad);
    // Add genlist items
    genlist_title_add(ad, "Time Range");
    for (int j = 0; j < 6; j++)
        genlist_radio_add(ad,
                          timeline_data[j].name,
                          goto_hash,
                          timeline_set_range,
                          (void *)j);
    genlist_padding_add(ad);
    // Refresh webview on naviframe pop
    elm_naviframe_item_pop_cb_set(nf_it, (Elm_Naviframe_Item_Pop_Cb)timeline_btn_pop_cb, ad);
}

void view_data_graph(int i)
{
    appdata_s *ad = get_appdata(NULL);
    curr_i = i;
    time_range_i = 0;
    // Generate new HTML string into memory
    graph_html = NULL;
    graph_html = new_string_printf(graph_html_template,
                                   ad->coin_data[i].symbol,
                                   ad->currency,
                                   ad->data_provider,
                                   ad->coin_data[i].symbol,
                                   ad->currency,
                                   (int)((double)ad->width * 0.9),
                                   (int)((double)ad->height * 0.9),
                                   timeline_data[time_range_i].data);
    // Create layout to hold ui elements
    Evas_Object *layout;
    layout = elm_layout_add(ad->naviframe);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_layout_theme_set(layout, "layout", "bottom_button", "default");
    // Create webview
    Evas *evas = evas_object_evas_get(layout);
    ad->web_view = ewk_view_add(evas);
    elm_object_part_content_set(layout, "elm.swallow.content", ad->web_view);
    evas_object_color_set(ad->web_view, __RGBA(0, 0, 0, 1));
    ewk_view_html_string_load(ad->web_view, graph_html, NULL, NULL);
    // Create timeline button
    ad->web_view_btn = elm_button_add(layout);
    elm_object_style_set(ad->web_view_btn, "bottom");
    evas_object_scale_set(ad->web_view_btn, 0.5);
    elm_object_text_set(ad->web_view_btn, timeline_data[time_range_i].name);
    elm_object_part_content_set(layout, "elm.swallow.button", ad->web_view_btn);
    evas_object_smart_callback_add(ad->web_view_btn, "clicked", timeline_btn_clicked_cb, NULL);
    evas_object_show(ad->web_view_btn);
    // Create parent layout
    Evas_Object *parent;
    parent = elm_layout_add(ad->naviframe);
    evas_object_size_hint_weight_set(parent, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_layout_theme_set(parent, "layout", "drawer", "panel");
    // Setup more option menu
    ad->more_option = eext_more_option_add(parent);
    view_data_populate_more_option(i);
    // Add items to parent
    elm_object_part_content_set(parent, "elm.swallow.content", layout);
    elm_object_part_content_set(parent, "elm.swallow.right", ad->more_option);
    // Callback to when more option is opened - prevent webview disappearing
    evas_object_smart_callback_add(ad->more_option, "more,option,closed", timeline_more_option_closed_cb, NULL);
    // Push webview to naviframe
    Elm_Object_Item *nf_it = NULL;
    nf_it = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, parent, "empty");
    elm_naviframe_item_pop_cb_set(nf_it, (Elm_Naviframe_Item_Pop_Cb)view_balance_data_pop_cb, NULL);
}

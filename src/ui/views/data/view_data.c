#include <app_resource_manager.h>
#include <cairo.h>
#include "main.h"
#include "../../draw.h"
#include "view_data_main.h"
#include "view_data_graph.h"
#include "view_data_price.h"
#include "view_data_volume.h"
#include "view_data_news.h"
#include "view_data.h"

static const MoreOptionData option_data[] = {
    {"Balance", "assets/more_option/balance.png", view_data_main},
    {"Graphs", "assets/more_option_price.png", view_data_graph},
    {"Price", "assets/more_option/price.png", view_data_price},
    {"Volume", "assets/more_option_balance.png", view_data_volume},
    {"News", "assets/more_option/news.png", view_data_news}};
static const int option_data_size = sizeof(option_data) / sizeof(option_data[0]);

// Cleanup on naviframe pop
static Eina_Bool view_data_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = data;
    // Free cairo resources
    cairo_destroy(ad->cairo);
    cairo_surface_destroy(ad->cairo_surface);
    return EINA_TRUE;
}

// Init new data view
void view_data_init(appdata_s *ad)
{
    // Init cairo and image
    cairo_image_new(ad);
    // Init drawing
    cairo_set_line_width(ad->cairo, VIEW_DATA_PRIMARY_LINE_WIDTH);
    cairo_set_source_rgba(ad->cairo, VIEW_DATA_BACKGROUND_COLOR);
}

// Called when 'more option' menu item is selected
void more_option_opened_cb(void *data, Evas_Object *obj, void *event_info)
{
    appdata_s *ad = get_appdata(NULL);
    int _i = (int)data;
    Eext_Object_Item *item = (Eext_Object_Item *)event_info;
    const char *text = eext_more_option_item_part_text_get(item, "selector,main_text");
    // Loop through array until matching text found; don't blame me, blame Tizen
    void (*cb)(int j) = NULL;
    for (int i = 0; i < option_data_size; i++)
    {
        if (!strcmp(text, option_data[i].text))
        {
            cb = option_data[i].cb;
            break;
        }
    }
    // Close more option menu
    eext_more_option_opened_set(obj, EINA_FALSE);
    // Pop current view from naviframe
    elm_naviframe_item_pop(ad->naviframe);
    // Run callback - open selected view
    if (cb != NULL)
        cb(_i);
}

void view_data_populate_more_option(int i)
{
    appdata_s *ad = get_appdata(NULL);
    Eext_Object_Item *item;
    Evas_Object *img;
    char *img_path;
    for (int j = 0; j < option_data_size; j++)
    {
        // Add new item
        item = eext_more_option_item_append(ad->more_option);
        // Set item text
        eext_more_option_item_part_text_set(item, "selector,main_text", option_data[j].text);
        // Set item image
        app_resource_manager_get(APP_RESOURCE_TYPE_IMAGE, option_data[j].img, &img_path);
        img = elm_image_add(ad->more_option);
        evas_object_size_hint_min_set(img, 76, 76);
        elm_image_file_set(img, img_path, NULL);
        eext_more_option_item_part_content_set(item, "item,icon", img);
        // Cleanup
        free(img_path);
    }
    evas_object_smart_callback_add(ad->more_option, "item,clicked", more_option_opened_cb, (const void *)i);
}

// Push view to naviframe
Elm_Object_Item *view_data_push(int i)
{
    appdata_s *ad = get_appdata(NULL);
    // Create new layout
    Evas_Object *layout;
    layout = elm_layout_add(ad->naviframe);
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_layout_theme_set(layout, "layout", "drawer", "panel");
    // Setup more option menu
    ad->more_option = eext_more_option_add(layout);
    view_data_populate_more_option(i);
    // Add items to layout
    elm_object_part_content_set(layout, "elm.swallow.content", ad->image);
    elm_object_part_content_set(layout, "elm.swallow.right", ad->more_option);
    // Push layout to naviframe
    Elm_Object_Item *nf_it = NULL;
    nf_it = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, layout, "empty");
    elm_naviframe_item_pop_cb_set(nf_it, (Elm_Naviframe_Item_Pop_Cb)view_data_pop_cb, ad);
    return nf_it;
}

// Setup text drawing parameters
void view_data_text_setup(cairo_t *cairo)
{
    cairo_select_font_face(cairo,
                           VIEW_DATA_FONT_FACE,
                           VIEW_DATA_FONT_SLANT,
                           VIEW_DATA_FONT_WEIGHT);
    cairo_set_font_size(cairo, VIEW_DATA_PRIMARY_FONT_SIZE);
    cairo_set_source_rgba(cairo, VIEW_DATA_TEXT_COLOR);
}

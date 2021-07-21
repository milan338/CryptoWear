#include "main.h"
#include "popup_view.h"

static void show_popup_view(appdata_s *ad, char *title, char *text, char *style)
{
    // Create popup object
    ad->popup = elm_popup_add(ad->naviframe);
    // Set style of popup
    evas_object_size_hint_weight_set(ad->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_popup_align_set(ad->popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
    elm_object_style_set(ad->popup, "circle");
    // Set popup content
    Evas_Object *layout;
    layout = elm_layout_add(ad->popup);
    elm_layout_theme_set(layout, "layout", "popup", style);
    elm_object_content_set(ad->popup, layout);
    // Set text
    elm_object_part_text_set(layout, "elm.text.title", title);
    elm_object_part_text_set(layout, "elm.text", text);
}

Elm_Object_Item *show_popup_view_ok(appdata_s *ad, char *title, char *text, void *cb, void *cb_data)
{
    // Create base view
    show_popup_view(ad, title, text, "content/circle/buttons1");
    // Button
    Evas_Object *btn;
    btn = elm_button_add(ad->popup);
    elm_object_style_set(btn, "popup/circle");
    evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(btn, "Ok");
    if (cb != NULL)
        evas_object_smart_callback_add(btn, "clicked", cb, cb_data);
    // Set buttons to action area
    elm_object_part_content_set(ad->popup, "button1", btn);
    // Push popup view to naviframe
    Elm_Object_Item *nf_it = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->popup, "empty");
    return nf_it;
}

void show_popup_view_yesno(appdata_s *ad, char *title, char *text, void *cb, void *ok_cb_data, void *cancel_cb_data)
{
    // Create base view
    show_popup_view(ad, title, text, "content/circle/buttons2");
    // Ok button
    Evas_Object *btn_ok;
    btn_ok = elm_button_add(ad->popup);
    elm_object_style_set(btn_ok, "popup/circle/right");
    evas_object_size_hint_weight_set(btn_ok, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    image_content_set(btn_ok, "assets/icon/tick.png");
    if (cb != NULL)
        evas_object_smart_callback_add(btn_ok, "clicked", cb, ok_cb_data);
    // Cancel button
    Evas_Object *btn_cancel;
    btn_cancel = elm_button_add(ad->popup);
    elm_object_style_set(btn_cancel, "popup/circle/left");
    evas_object_size_hint_weight_set(btn_cancel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    image_content_set(btn_cancel, "assets/icon/cross.png");
    if (cb != NULL)
        evas_object_smart_callback_add(btn_cancel, "clicked", cb, cancel_cb_data);
    // Set buttons to action area
    elm_object_part_content_set(ad->popup, "button1", btn_cancel);
    elm_object_part_content_set(ad->popup, "button2", btn_ok);
    // Push popup view to naviframe
    elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->popup, "empty");
}

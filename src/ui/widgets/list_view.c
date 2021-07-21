#include <app_resource_manager.h>
#include <bundle.h>
#include "main.h"
#include "../../rw/file.h"
#include "../../rw/web.h"
#include "list_view.h"

// Get text for genlist line
static char *item_label_get(void *data, Evas_Object *obj, const char *part)
{
    if (!strcmp(part, "elm.text"))
    {
        char *label = data;
        return strdup(label);
    }
    else
        return NULL;
}

// Get text for genlist line with subtext representing current application setting
static char *item_setting_get(void *data, Evas_Object *obj, const char *part)
{
    if (!strcmp(part, "elm.text"))
    {
        char *label = data;
        return strdup(label);
    }
    else if (!strcmp(part, "elm.text.1"))
    {
        char *label = NULL;
        char *subtext = NULL;
        // Convert label to lowercase settings key
        Eina_Strbuf *strbuf = eina_strbuf_new();
        eina_strbuf_append(strbuf, (char *)data);
        eina_strbuf_tolower(strbuf);
        label = eina_strbuf_string_steal(strbuf);
        eina_strbuf_free(strbuf);
        // Get settings value from lowercase label
        get_preference_char(label, &subtext);
        // Caller will free resources
        return subtext;
    }
    else
        return NULL;
}

// Get text for refresh button
static char *item_refresh_get(void *data, Evas_Object *obj, const char *part)
{
    appdata_s *ad = get_appdata(NULL);
    if (!strcmp(part, "elm.text"))
    {
        char *label = data;
        return strdup(label);
    }
    else if (!strcmp(part, "elm.text.1"))
        return unix_time_fmt((time_t)ad->last_updated);
    else
        return NULL;
}

// Get text for crypto item
static char *item_crypto_get_text(void *data, Evas_Object *obj, const char *part)
{
    char *label = data;
    char *name = NULL;
    appdata_s *ad = get_appdata(NULL);
    eina_lock_take(&ad->coin_list_mutex);
    int err = bundle_get_str(ad->crypto_names, label, &name);
    eina_lock_release(&ad->coin_list_mutex);
    // Return crypto name if it exists, otherwise return the code
    if (!strcmp(part, "elm.text"))
    {
        if (err != BUNDLE_ERROR_NONE)
            return strdup(label);
        else
            return strdup(name);
    }
    // Return crypto code if name passed for main text, otherwise return nothing
    else if (!strcmp(part, "elm.text.1"))
    {
        if (err != BUNDLE_ERROR_NONE)
            return NULL;
        else
            return strdup(label);
    }
    else
        return NULL;
}

// Get text for fiat currency item
static char *item_fiat_get_text(void *data, Evas_Object *obj, const char *part)
{
    char *label = data;
    FiatData *fiat_data = NULL;
    size_t size;
    appdata_s *ad = get_appdata(NULL);
    eina_lock_take(&ad->fiat_data_mutex);
    bundle_error_e err = bundle_get_byte(ad->fiat_data, label, (void **)&fiat_data, &size);
    eina_lock_release(&ad->fiat_data_mutex);
    // Return fiat name if it exists, otherwise return the code
    if (!strcmp(part, "elm.text"))
    {
        if (err != BUNDLE_ERROR_NONE)
            return strdup(label);
        else
            return strdup(fiat_data->name);
    }
    // Return fiat code if name passed for main text, otherwise returl nothing
    else if (!strcmp(part, "elm.text.1"))
    {
        if (err != BUNDLE_ERROR_NONE)
            return NULL;
        else
            return strdup(label);
    }
    else
        return NULL;
}

// Get text for news item
static char *item_news_get_text(void *data, Evas_Object *obj, const char *part)
{
    appdata_s *ad = get_appdata(NULL);
    char *ret = NULL;
    eina_lock_take(&ad->coin_news_mutex);
    CoinNewsData *news = data;
    if (!strcmp(part, "elm.text"))
        ret = strdup(news->title);
    else if (!strcmp(part, "elm.text.1"))
        ret = strdup(news->source);
    else if (!strcmp(part, "elm.text.2"))
        ret = unix_hours_ago(news->published_on);
    eina_lock_release(&ad->coin_news_mutex);
    return ret;
}

// Get content for genlist line with radio button
static Evas_Object *item_radio_get(void *data, Evas_Object *obj, const char *part)
{
    appdata_s *ad = get_appdata(NULL);
    Evas_Object *radio = NULL;
    char *label = data;
    int radio_id;
    if (!strcmp(part, "elm.icon"))
    {
        radio = elm_radio_add(obj);
        // Turn label into unique radio id hash
        radio_id = eina_hash_superfast(label, strlen(label));
        elm_radio_state_value_set(radio, radio_id);
        // Add radio to root group
        elm_radio_group_add(radio, ad->radio_group);
    }
    return radio;
}

// Get content for genlist line with crypto icon
static Evas_Object *item_crypto_get_content(void *data, Evas_Object *obj, const char *part)
{
    appdata_s *ad = get_appdata(NULL);
    Evas_Object *icon = NULL;
    Eina_Bool ret;
    char *label = data;
    char *file_name = NULL;
    char *icon_path = NULL;
    if (!strcmp(part, "elm.icon"))
    {
        // Create new image
        icon = elm_image_add(obj);
        // Create new filepath based on crypto label
        file_name = new_string_printf("%s.png", label);
        icon_path = appdata_get_shared_filepath(file_name);
        int *status = NULL;
        size_t status_size;
        // Get file availability status
        int file_exists = 0;
        eina_lock_take(&ad->file_status_mutex);
        bundle_get_byte(ad->file_status, icon_path, (void **)&status, &status_size);
        file_exists = *status;
        eina_lock_release(&ad->file_status_mutex);
        if (file_exists)
            ret = elm_image_file_set(icon, icon_path, NULL);
        else
            ret = EINA_FALSE;
        // Image not found
        if (ret != EINA_TRUE)
        {
            free(icon_path);
            // Use generic image
            app_resource_manager_get(APP_RESOURCE_TYPE_IMAGE, "assets/coin_icon/crypto_generic.png", &icon_path);
            // Set image file
            elm_image_file_set(icon, icon_path, NULL);
        }
        // Default icon size for genlist item
        evas_object_size_hint_min_set(icon, 76, 76);
        evas_object_show(icon);
        // Cleanup
        free(file_name);
        free(icon_path);
    }
    return icon;
}

// Add item to genlist
static Elm_Widget_Item *genlist_item_add(appdata_s *ad, const Elm_Genlist_Item_Class *itc, const void *data, void *callback, void *cb_data)
{
    Elm_Widget_Item *it = elm_genlist_item_append(ad->genlist,
                                                  itc,
                                                  data,
                                                  NULL,
                                                  ELM_GENLIST_ITEM_NONE,
                                                  callback,
                                                  cb_data);
    return it;
}

// Add title to genlist
void genlist_title_add(appdata_s *ad, const char *label)
{
    genlist_item_add(ad, &ad->genlist_title_class, label, NULL, NULL);
}

// Add divider with text to genlist
void genlist_groupindex_add(appdata_s *ad, const char *label)
{
    genlist_item_add(ad, &ad->genlist_groupindex_class, label, NULL, NULL);
}

// Add user-selectable line to genlist
void genlist_line_add(appdata_s *ad, const char *label, void *callback, void *cb_data)
{
    genlist_item_add(ad, &ad->genlist_line_class, label, callback, cb_data);
}

// Add multiline news to genlist
void genlist_news_add(appdata_s *ad, CoinNewsData *data, void *callback, void *cb_data)
{
    genlist_item_add(ad, &ad->genlist_news_class, data, callback, cb_data);
}

// Add user-selectable line with radio button indicator to genlist
void genlist_radio_add(appdata_s *ad, const char *label, int goto_hash, void *callback, void *cb_data)
{
    Elm_Widget_Item *it = genlist_item_add(ad, &ad->genlist_radio_class, label, callback, cb_data);
    // Open genlist with this radio in the centre if it is currently selected
    if (eina_hash_superfast(label, strlen(label)) == goto_hash)
        elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
}

// Add user-selectable line to genlist with subtext below representing setting data based on the label
Elm_Widget_Item *genlist_setting_add(appdata_s *ad, const char *label, void *callback, void *cb_data)
{
    Elm_Widget_Item *it = genlist_item_add(ad, &ad->genlist_setting_class, label, callback, cb_data);
    return it;
}

// Add user-selectable line with last refresh time
Elm_Widget_Item *genlist_refresh_add(appdata_s *ad, const char *label, void *callback, void *cb_data)
{
    Elm_Widget_Item *it = genlist_item_add(ad, &ad->genlist_refresh_class, label, callback, cb_data);
    return it;
}

// Add user-selectable line to genlist with label representing crypto name and subtext representing crypto code
void genlist_crypto_add(appdata_s *ad, const char *label, void *callback, void *cb_data)
{
    genlist_item_add(ad, &ad->genlist_crypto_class, label, callback, cb_data);
}

// Add user-selectable line to genlist with radio button indicator, fiat name as text, and fiat code as subtext
void genlist_fiat_add(appdata_s *ad, const char *label, int goto_hash, void *callback, void *cb_data)
{
    Elm_Widget_Item *it = genlist_item_add(ad, &ad->genlist_fiat_class, label, callback, cb_data);
    // Open genlist with this radio in the centre if it is currently selected
    if (eina_hash_superfast(label, strlen(label)) == goto_hash)
        elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
}

// Add padding to genlist
void genlist_padding_add(appdata_s *ad)
{
    genlist_item_add(ad, &ad->genlist_padding_class, NULL, NULL, NULL);
}

// Create item classes for genlist
void create_genlist_item_classes(appdata_s *ad)
{
    // Title definitions
    ad->genlist_title_class.item_style = "title";
    ad->genlist_title_class.func.text_get = item_label_get;
    // Groupindex definitions
    ad->genlist_groupindex_class.item_style = "groupindex";
    ad->genlist_groupindex_class.func.text_get = item_label_get;
    // Line definitions
    ad->genlist_line_class.item_style = "1text";
    ad->genlist_line_class.func.text_get = item_label_get;
    // Radio button definitions
    ad->genlist_radio_class.item_style = "1text.1icon.1";
    ad->genlist_radio_class.func.text_get = item_label_get;
    ad->genlist_radio_class.func.content_get = item_radio_get;
    // Line with setting value as sub-text definitions
    ad->genlist_setting_class.item_style = "2text";
    ad->genlist_setting_class.func.text_get = item_setting_get;
    // Line with last refresh time as sub-text definitions
    ad->genlist_refresh_class.item_style = "2text";
    ad->genlist_refresh_class.func.text_get = item_refresh_get;
    // Line with crypto icon
    ad->genlist_crypto_class.item_style = "2text.1icon";
    ad->genlist_crypto_class.func.text_get = item_crypto_get_text;
    ad->genlist_crypto_class.func.content_get = item_crypto_get_content;
    // Line with news
    ad->genlist_news_class.item_style = "3text";
    ad->genlist_news_class.func.text_get = item_news_get_text;
    // Radio button with fiat currency data
    ad->genlist_fiat_class.item_style = "2text.1icon.1";
    ad->genlist_fiat_class.func.text_get = item_fiat_get_text;
    ad->genlist_fiat_class.func.content_get = item_radio_get;
    // Padding definitions
    ad->genlist_padding_class.item_style = "padding";
}

// Show circle genlist
Elm_Object_Item *show_list_view(appdata_s *ad)
{
    // Create genlist
    ad->genlist = elm_genlist_add(ad->naviframe);
    // Don't run callbacks for all items in advance
    elm_genlist_homogeneous_set(ad->genlist, EINA_TRUE);
    // Set genlist mode to scroll text horizontally if too wide
    elm_genlist_mode_set(ad->genlist, ELM_LIST_SCROLL);
    // Create circle genlist
    ad->circle_genlist = eext_circle_object_genlist_add(ad->genlist, ad->circle_suface);
    eext_circle_object_genlist_scroller_policy_set(ad->circle_genlist,
                                                   ELM_SCROLLER_POLICY_OFF,
                                                   ELM_SCROLLER_POLICY_AUTO);
    // Activate rotary event for scrolling
    eext_rotary_object_event_activated_set(ad->circle_genlist, EINA_TRUE);
    // Push list view to naviframe
    Elm_Object_Item *nf_it = NULL;
    nf_it = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->genlist, "empty");
    return nf_it;
}

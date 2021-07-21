#include <EWebKit.h>
#include "main.h"
#include "article_view.h"

static char *article_html;
static char *article_published_on;

static Eina_Bool article_view_pop_cb(void *data, Evas_Object *obj, void *event_info)
{
    // Close webview instance
    ewk_shutdown();
    // Cleanup
    free(article_html);
    free(article_published_on);
    return EINA_TRUE;
}

void show_article_view(CoinNewsData *news)
{
    appdata_s *ad = get_appdata(NULL);
    // Generate new HTML string into memory
    article_html = NULL;
    eina_lock_take(&ad->coin_news_mutex);
    article_published_on = unix_time_fmt(news->published_on);
    article_html = new_string_printf(article_html_template,
                                     news->title,
                                     news->source,
                                     article_published_on,
                                     news->body,
                                     news->url);
    eina_lock_release(&ad->coin_news_mutex);
    // Create webview
    Evas *evas = evas_object_evas_get(ad->naviframe);
    ad->web_view = ewk_view_add(evas);
    evas_object_color_set(ad->web_view, __RGBA(0, 0, 0, 1));
    ewk_view_html_string_load(ad->web_view, article_html, NULL, NULL);
    // Push webview to naviframe
    Elm_Object_Item *nf_it = NULL;
    nf_it = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->web_view, "empty");
    elm_naviframe_item_pop_cb_set(nf_it, (Elm_Naviframe_Item_Pop_Cb)article_view_pop_cb, NULL);
}

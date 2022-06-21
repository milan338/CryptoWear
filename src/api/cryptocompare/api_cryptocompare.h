#ifndef APICRYPTOCOMPARE_H
#define APICRYPTOCOMPARE_H

#include <bundle.h>
#include "main.h"

#define API_CRYPTOCOMPARE_COINLIST "https://min-api.cryptocompare.com/data/all/coinlist?summary=true"
#define API_CRYPTOCOMPARE_NEWS "https://min-api.cryptocompare.com/data/v2/news/?categories=%s&excludeCategories=Sponsored"
#define API_CRYPTOCOMPARE_FULL "https://min-api.cryptocompare.com/data/pricemultifull?fsyms=%s&tsyms=%s"

struct ApiFullData
{
    char *from;
    bundle *coin_data;
} ApiFullData;

void api_cryptocompare_coinlist_thread_cb(void *data, Ecore_Thread *thread);

void api_cryptocompare_news(appdata_s *ad, char *coin, void *finished_cb, void *cancelled_cb);

void api_cryptocompare_full_async(char *from, bundle *coin_data, void *finished_cb);
void api_cryptocompare_full_sync(char *from, bundle *coin_data);

#endif // APICRYPTOCOMPARE_H

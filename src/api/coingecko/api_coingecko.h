#ifndef APICOINGECKO_H
#define APICOINGECKO_H

#include "main.h"

#define API_COINGECKO_COIN_LIST "https://api.coingecko.com/api/v3/coins/list?include_platform=false"

void api_coingecko_coin_list(appdata_s *ad);
void api_coingecko_coin_list_thread_cb(void *data, Ecore_Thread *thread);

#endif // APICOINGECKO_H

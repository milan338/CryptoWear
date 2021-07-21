#ifndef APICOINBASE_H
#define APICOINBASE_H

#include "main.h"

#define API_COINBASE_URL_BASE "https://api.coinbase.com"
#define API_COINBASE_PATH_BASE "/v2/accounts"
#define API_COINBASE_VERSION "2021-03-23"

void api_coinbase(void *data, Ecore_Thread *thread);

#endif // APICOINBASE_H

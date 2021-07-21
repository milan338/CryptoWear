#ifndef VIEWEXCHANGES_H
#define VIEWEXCHANGES_H

#include "main.h"

typedef struct exchange_views_s
{
    const char *exchange;
    const char *label;
    const char *bundle_key;
    const char *_key;
    const char *_secret;
    void *api_cb;
} exchange_views_s;

void view_exchanges_all(appdata_s *ad);
void view_exchanges_my(appdata_s *ad);

#endif // VIEWEXCHANGES_H

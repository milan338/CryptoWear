#ifndef __cryptowear_H__
#define __cryptowear_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#include <bundle.h>

#include "util.h"
#include "appdata.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "cryptowear"

#if !defined(PACKAGE)
#define PACKAGE "com.milan338.cryptowear"
#endif

typedef struct CoinIcon
{
    char *coin;
    char *path;
} CoinIcon;

static const CoinIcon local_icons[] = {
    {"XLM", "assets/coin_icon/xlm.png"},
    {"XRP", "assets/coin_icon/xrp.png"}};
static const int local_icons_size = sizeof(local_icons) / sizeof(local_icons[0]);

appdata_s *get_appdata(appdata_s *ad);

#endif /* __cryptowear_H__ */

#ifndef VIEWSETTINGS_H
#define VIEWSETTINGS_H

#include "main.h"

static const char *data_providers_list[] = {
    "Binance",
    "Bingbon",
    "BitBay",
    "BitCoke",
    "Bitfinex",
    "bitFlyer",
    "Bithumb",
    "BITKUB",
    "BitMax",
    "BitMEX",
    "Bitpanda Pro",
    "BTSE",
    "Bitso",
    "Bitstamp",
    "Bittrex",
    "BTCYou",
    "Bybit",
    "CEX.IO",
    "Coinbase",
    "Coinfloor",
    "Currencycom",
    "Deribit",
    "FTX",
    "Gemini",
    "HitBTC",
    "Huobi",
    "KORBIT",
    "Kraken",
    "KuCoin",
    "Mercado",
    "OKCoin",
    "OKEx",
    "Phemex",
    "Poloniex",
    "The Rock Trading",
    "TimeX",
    "TradeStation",
    "UNISWAP"};
#define DATA_PROVIDER_NUM (sizeof(data_providers_list) / sizeof(data_providers_list[0]))

void view_settings(appdata_s *ad);

#endif // VIEWSETTINGS_H

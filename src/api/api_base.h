#ifndef APIBASE_H
#define APIBASE_H

#include <json-glib/json-glib.h>
#include "main.h"

void download_crypto_icon(char *crypto);

char *sign_hmac_string(const char *message, const char *key, GChecksumType digest_type);

char *random_str(int len);

#endif // APIBASE_H

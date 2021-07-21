#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "main.h"

typedef struct PreferenceData
{
    char *key;
    void *default_val;
    void *to_var;
    void *cb;
    void *cb_data;
} PreferenceData;

void init_preferences(appdata_s *ad);

#endif // PREFERENCES_H

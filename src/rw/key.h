#ifndef KEY_H
#define KEY_H

#include "main.h"

int remove_secure_data(char *key);

int save_secure_data_overwrite(char *key, void *data, size_t size);
int save_secure_data(char *key, void *data, size_t size);

void *get_secure_data(char *key);

#endif // KEY_H

#ifndef FILE_H
#define FILE_H

#include "main.h"

char *appdata_get_shared_filepath(char *file_name);

void update_file_status_bundle(char *path, int status);

#endif // FILE_H

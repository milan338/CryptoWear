#include <ckmc/ckmc-manager.h>
#include "main.h"
#include "key.h"

// Remove data securely from the key store
int remove_secure_data(char *key)
{
    int ret = ckmc_remove_alias(key);
    return ret;
}

// Save data securely to the key store, overwriting any existing data
int save_secure_data_overwrite(char *key, void *data, size_t size)
{
    // Try removing any existing data
    ckmc_remove_alias(key);
    // Save new data
    int ret = save_secure_data(key, data, size);
    return ret;
}

// Save data securely to the key store
int save_secure_data(char *key, void *data, size_t size)
{
    // Data buffer to save
    ckmc_raw_buffer_s buffer = {(unsigned char *)data, size};
    // Save policy - no password protection, can extract raw value
    ckmc_policy_s policy = {NULL, true};
    // Save the data
    int ret = ckmc_save_data(key, buffer, policy);
    return ret;
}

// Retrieve data securely from the key store
// Returned value must be freed if not NULL, otherwise failed to retreive data
void *get_secure_data(char *key)
{
    // Data buffer to fill
    ckmc_raw_buffer_s *buffer = NULL;
    // Get the data
    int ret = ckmc_get_data(key, NULL, &buffer);
    // Successfully got data
    if (ret == CKMC_ERROR_NONE)
    {
        // Get data into void pointer
        void *data = malloc(buffer->size);
        memcpy(data, (const void *)buffer->data, buffer->size);
        ckmc_buffer_free(buffer);
        return data;
    }
    // Failed to get data
    else
        return NULL;
}

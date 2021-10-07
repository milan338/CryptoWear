#include <net_connection.h>
#include "main.h"
#include "network.h"

static connection_h connection = NULL;

// Create new global connection
// destroy_existing_connection must be called once no longer using the connection
int create_new_connection()
{
    appdata_s *ad = get_appdata(NULL);
    connection_type_e conn_type;
    int err = connection_create(&connection);
    if (err == CONNECTION_ERROR_NONE)
    {
        connection_get_type(connection, &conn_type);
        // Connected through proxy
        if (conn_type == CONNECTION_TYPE_NET_PROXY || conn_type == CONNECTION_TYPE_ETHERNET)
        {
            int ret = connection_get_proxy(connection, CONNECTION_ADDRESS_FAMILY_IPV4, &ad->proxy_addr);
            if (ret != CONNECTION_ERROR_NONE)
                ad->proxy_addr = NULL;
        }
    }
    return err;
}

// Destroy existing global connection
int destroy_existing_connection()
{
    appdata_s *ad = get_appdata(NULL);
    int err = connection_destroy(connection);
    connection = NULL;
    // Free existing proxy address
    if (ad->proxy_addr != NULL)
    {
        free(ad->proxy_addr);
        ad->proxy_addr = NULL;
    }
    return err;
}

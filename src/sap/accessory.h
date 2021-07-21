#ifndef ACCESSORY_H
#define ACCESSORY_H

#include <sap.h>
#include "main.h"

#define ACCESSORY_PROFILE_ID "/com/milan338/cryptowear"
#define ACCESSORY_CHANNEL_ID 169

typedef struct SAPData
{
    sap_agent_h agent;
    sap_socket_h socket;
    sap_peer_agent_h peer_agent;
} SAPData;

typedef enum sap_init_code
{
    SAP_INIT_AGENT_ERR,
    SAP_INIT_SUCCESS,
    SAP_INIT_AGENT_NOT_INIT,
} sap_init_code_e;

void create_service_connection(void *data, Ecore_Thread *thread);
void terminate_service_connection(void *data, Ecore_Thread *thread);
bool find_peer_agent(void *data, Ecore_Thread *thread);

bool companion_send_secure_data(char *data);

int sap_init(appdata_s *ad);
void sap_shutdown();

#endif // ACCESSORY_H

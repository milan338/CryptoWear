#include <sap.h>
#include "main.h"
#include "../ui/widgets/popup_view.h"
#include "receive_data.h"
#include "accessory.h"

static bool agent_created = false;

static SAPData sap_data = {0};

// Terminate an existing connection upon failure and alert user
static void connection_fail(char *reason)
{
    appdata_s *ad = get_appdata(NULL);
    show_popup_view_ok(ad, "Failed to Sync", reason, elm_naviframe_item_pop, ad->naviframe);
    terminate_service_connection(NULL, NULL);
}

// Called when SAP agent is init
static void on_agent_initialized(sap_agent_h agent, sap_agent_initialized_result_e result, void *user_data)
{
    switch (result)
    {
    case SAP_AGENT_INITIALIZED_RESULT_SUCCESS:
        sap_data.agent = agent;
        agent_created = true;
        break;
    case SAP_AGENT_INITIALIZED_RESULT_DUPLICATED:
        connection_fail("Attempted to connect to an existing connection");
        break;
    case SAP_AGENT_INITIALIZED_RESULT_INVALID_ARGUMENTS:
        connection_fail("Invalid connection arguments");
        break;
    case SAP_AGENT_INITIALIZED_RESULT_INTERNAL_ERROR:
        connection_fail("Internal error");
        break;
    // Unknown status
    default:
        connection_fail("Unknown status");
        break;
    }
}

// Called when SAP peer agent is updated
static void on_peer_agent_updated(sap_peer_agent_h peer_agent, sap_peer_agent_status_e peer_status, sap_peer_agent_found_result_e result, void *user_data)
{
    switch (result)
    {
    case SAP_PEER_AGENT_FOUND_RESULT_DEVICE_NOT_CONNECTED:
        connection_fail("Device not connected");
        break;
    case SAP_PEER_AGENT_FOUND_RESULT_FOUND:
        // Ensure peer agent is valid
        if (peer_status == SAP_PEER_AGENT_STATUS_AVAILABLE)
        {
            // Save current connect peer agent
            sap_data.peer_agent = peer_agent;
            // Request SAP connection to peer
            create_service_connection(user_data, NULL);
        }
        // Unavailable peer agent
        else
        {
            sap_peer_agent_destroy(peer_agent);
            connection_fail("Device unavailable");
        }
        break;
    case SAP_PEER_AGENT_FOUND_RESULT_SERVICE_NOT_FOUND:
        connection_fail("Device not supported");
        break;
    case SAP_PEER_AGENT_FOUND_RESULT_TIMEDOUT:
        connection_fail("Timed out waiting for device");
        break;
    case SAP_PEER_AGENT_FOUND_RESULT_INTERNAL_ERROR:
        connection_fail("Internal error");
        break;
    }
}

// Called when SAP connection terminated
static void on_service_connection_terminated(sap_peer_agent_h peer_agent, sap_socket_h socket, sap_service_connection_terminated_reason_e result, void *user_data)
{
    switch (result)
    {
    case SAP_CONNECTION_TERMINATED_REASON_PEER_DISCONNECTED:
        break;
    case SAP_CONNECTION_TERMINATED_REASON_DEVICE_DETACHED:
        break;
    case SAP_CONNECTION_TERMINATED_REASON_UNKNOWN:
        break;
    }
    // Remove SAP socket
    sap_socket_destroy(sap_data.socket);
    sap_data.socket = NULL;
}

// Called when device status is changed
static void on_device_status_changed(sap_device_status_e status, sap_transport_type_e transport_type, void *user_data)
{
    switch (transport_type)
    {
    case SAP_TRANSPORT_TYPE_BT:
        break;
    case SAP_TRANSPORT_TYPE_BLE:
        break;
    case SAP_TRANSPORT_TYPE_TCP:
        break;
    case SAP_TRANSPORT_TYPE_USB:
        break;
    case SAP_TRANSPORT_TYPE_MOBILE:
        break;
    // Unknown connection type
    default:
        break;
    }

    switch (status)
    {
    case SAP_DEVICE_STATUS_DETACHED:
        // Destroy any existing connection when device detached
        if (sap_data.peer_agent)
        {
            sap_socket_destroy(sap_data.socket);
            sap_data.socket = NULL;
            sap_peer_agent_destroy(sap_data.peer_agent);
            sap_data.peer_agent = NULL;
        }
        break;
    case SAP_DEVICE_STATUS_ATTACHED:
        // Find an available peer agent
        if (agent_created)
            ;
        break;
    // Uknown status
    default:
        break;
    }
}

// Called when data received from peer agent
// Buffer is duplicated and must eventually be freed
static void on_data_received(sap_socket_h socket, unsigned short int channel_id, unsigned int payload_length, void *buffer, void *user_data)
{
    char *data = strdup((char *)buffer);
    parse_keys_data(data, (size_t)payload_length);
    // Terminate connection to peer
    terminate_service_connection(NULL, NULL);
}

// Called when new SAP connection created
static void on_service_connection_created(sap_peer_agent_h peer_agent, sap_socket_h socket, sap_service_connection_result_e result, void *user_data)
{
    switch (result)
    {
    case SAP_CONNECTION_SUCCESS:
        // Function called when connection with peer is lost
        sap_peer_agent_set_service_connection_terminated_cb(
            sap_data.peer_agent,
            on_service_connection_terminated,
            NULL);
        // Function called when data received from peer
        sap_socket_set_data_received_cb(socket, on_data_received, peer_agent);
        // Update local socket
        sap_data.socket = socket;
        // Run callback function
        if (user_data != NULL)
        {
            void (*cb)() = user_data;
            cb();
        }
        break;
    case SAP_CONNECTION_ALREADY_EXIST:
        // Update local socket
        sap_data.socket = socket;
        break;
    case SAP_CONNECTION_FAILURE_DEVICE_UNREACHABLE:
        connection_fail("Device unreachable");
        break;
    case SAP_CONNECTION_FAILURE_INVALID_PEERAGENT:
        connection_fail("Invalid device");
        break;
    case SAP_CONNECTION_FAILURE_NETWORK:
        connection_fail("Network failure");
        break;
    case SAP_CONNECTION_FAILURE_PEERAGENT_NO_RESPONSE:
        connection_fail("No response from device");
        break;
    case SAP_CONNECTION_FAILURE_PEERAGENT_REJECTED:
        connection_fail("Device rejected connection");
        break;
    case SAP_CONNECTION_IN_PROGRESS:
        break;
    case SAP_CONNECTION_PEER_AGENT_NOT_SUPPORTED:
        connection_fail("Device not supported");
        break;
    case SAP_CONNECTION_FAILURE_UNKNOWN:
        connection_fail("Unknown error");
        break;
    }
}

// Thread - connect to SAP peer
void create_service_connection(void *data, Ecore_Thread *thread)
{
    // Request SAP connection to peer
    int ret = sap_agent_request_service_connection(
        sap_data.agent,
        sap_data.peer_agent,
        on_service_connection_created,
        data);
    if (ret != SAP_RESULT_SUCCESS)
        connection_fail("Failed to connect to device");
}

// Thread - terminate SAP peer connection
void terminate_service_connection(void *data, Ecore_Thread *thread)
{
    appdata_s *ad = get_appdata(NULL);
    int ret = SAP_RESULT_FAILURE;
    // Terminate connection only if already established
    if (sap_data.socket)
        ret = sap_peer_agent_terminate_service_connection(sap_data.peer_agent);
    else
        return;
    if (ret != SAP_RESULT_SUCCESS)
        show_popup_view_ok(ad, "Failed to Sync", "Failed to terminate device connection", elm_naviframe_item_pop, ad->naviframe);
}

// Thread - find SAP peer agent, returns false if no agent found
bool find_peer_agent(void *data, Ecore_Thread *thread)
{
    // Find peer agent with same ID as agent
    int ret = sap_agent_find_peer_agent(sap_data.agent, on_peer_agent_updated, data);
    if (ret != SAP_RESULT_SUCCESS)
    {
        connection_fail("No devices found");
        return false;
    }
    return true;
}

// Send data to peer, returns false if socket non-existent or error occurred
bool companion_send_secure_data(char *data)
{
    bool ret = false;
    int err;
    // Ensure socket exists
    if (sap_data.socket)
    {
        err = sap_socket_send_secure_data(sap_data.socket, ACCESSORY_CHANNEL_ID, strlen(data), data);
        ret = err == SAP_SOCKET_RESULT_SUCCESS;
    }
    return ret;
}

// Init SAP
int sap_init(appdata_s *ad)
{
    // Create new SAP agent
    sap_agent_h agent = NULL;
    sap_agent_create(&agent);
    if (agent == NULL)
        return SAP_INIT_AGENT_ERR;
    sap_data.agent = agent;
    // Set new callback for device status change
    sap_set_device_status_changed_cb(on_device_status_changed, NULL);
    // Init SAP agent
    int result;
    int i = 0;
    // Try init agent until max tries reached
    do
    {
        result = sap_agent_initialize(
            sap_data.agent,
            ACCESSORY_PROFILE_ID,
            SAP_AGENT_ROLE_CONSUMER,
            on_agent_initialized, NULL);
        i++;
    } while (result != SAP_RESULT_SUCCESS && i <= 10);
    if (i == 10)
    {
        show_popup_view_ok(ad, "SAP Failure", "Failed to init SAP service, keys cannot be synced", elm_naviframe_item_pop, ad->naviframe);
        return SAP_INIT_AGENT_NOT_INIT;
    }
    else
        return SAP_INIT_SUCCESS;
}

// Shutdown SAP
void sap_shutdown()
{
    // Destroy SAP agent
    if (sap_data.agent != NULL)
        sap_agent_destroy(sap_data.agent);
    // Destroy peer agent
    if (sap_data.peer_agent != NULL)
        sap_peer_agent_destroy(sap_data.peer_agent);
    // Destroy socket
    if (sap_data.socket != NULL)
        sap_socket_destroy(sap_data.socket);
}

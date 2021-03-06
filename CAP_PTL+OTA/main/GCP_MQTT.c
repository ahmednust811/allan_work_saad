/*#include "esp_log.h"
#include "JWT_MAIN.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include "base64url.h"*/
#include "main_task.h"
#include <iotc.h>
#include <iotc_error.h>
#include <iotc_jwt.h>
//#include "iot-device-sdk-embedded-c\examples\common\src\example_utils.h"
#include "iotc_types_internal.h"
#include "esp_log.h"
static const char TAG[]= "MQTT PART";
 char rub[] = "-----BEGIN EC PRIVATE KEY-----\nMHcCAQEEIEoaUviMrlozo/8r1hxl35J/TpD5Y02PvYRgryaqCq5ioAoGCCqGSM49AwEHoUQDQgAELJzbM7LLa5bQPNFQwU4NijTE5b5HiNflalFSZaH48D8yibi2fZcyY7vZJIk0Z78RZQIvWLsRyCJBozOSoXkCCA==\n-----END EC PRIVATE KEY-----\n";
//======================================================================<>================================================================================================================
iotc_mqtt_qos_t iotc_example_qos = IOTC_MQTT_QOS_AT_LEAST_ONCE;
static iotc_timed_task_handle_t delayed_publish_task =
    IOTC_INVALID_TIMED_TASK_HANDLE;
iotc_context_handle_t iotc_context = IOTC_INVALID_CONTEXT_HANDLE;




void publish_telemetry_event(iotc_context_handle_t context_handle,
                             iotc_timed_task_handle_t timed_task, void *user_data)
{
    IOTC_UNUSED(timed_task);
    IOTC_UNUSED(user_data);

    char *publish_topic = mqtt_values.mqtt_publish_link;//"/devices/A2639856316968383/events";//;
    //asprintf(&publish_topic, PUBLISH_TOPIC_EVENT, CONFIG_GIOT_DEVICE_ID);
    char *publish_message;
    asprintf(&publish_message,"[{\"ID\":%llu, \"DATE\": \"%s\", \"RPM\":%2f, \"REV\":%2f,\"SUM\":%2f,\"DIR\":%u,\"BTN\":%u,\"CUR\":%2f}]",mqtt_values.mqtt_project_id,"2022-03-27T18:42:32.2",15.6,0.07,0.07,2,2,33.84);
    //asprintf(&publish_message, TEMPERATURE_DATA, MIN_TEMP + rand() % 10);
    ESP_LOGI(TAG, "publishing msg \"%s\" to topic: \"%s\"", publish_message, publish_topic);

    iotc_publish(context_handle, publish_topic, publish_message,
                 iotc_example_qos,
                 /*callback=*/NULL, /*user_data=*/NULL);
    //free(publish_topic);
    //free(publish_message);
}



void on_connection_state_changed(iotc_context_handle_t in_context_handle,
                                 void *data, iotc_state_t state)
{
    iotc_connection_data_t *conn_data = (iotc_connection_data_t *)data;

    switch (conn_data->connection_state) {
    /* IOTC_CONNECTION_STATE_OPENED means that the connection has been
       established and the IoTC Client is ready to send/recv messages */
    case IOTC_CONNECTION_STATE_OPENED:
        ESP_LOGI(TAG, "connected!");
        /////////////publish_telemetry_event(in_context_handle);
        /* Create a timed task to publish every 10 seconds. */
        delayed_publish_task = iotc_schedule_timed_task(in_context_handle,
                               publish_telemetry_event, 3,
                               15, /*user_data=*/NULL);
        break;

    /* IOTC_CONNECTION_STATE_OPEN_FAILED is set when there was a problem
       when establishing a connection to the server. The reason for the error
       is contained in the 'state' variable. Here we log the error state and
       exit out of the application. */

    /* Publish immediately upon connect. 'publish_function' is defined
       in this example file and invokes the IoTC API to publish a
       message. */
    case IOTC_CONNECTION_STATE_OPEN_FAILED:
        ESP_LOGI(TAG, "ERROR! Connection has failed reason %d", state);

        /* exit it out of the application by stopping the event loop. */
        iotc_events_stop();
        break;

    /* IOTC_CONNECTION_STATE_CLOSED is set when the IoTC Client has been
       disconnected. The disconnection may have been caused by some external
       issue, or user may have requested a disconnection. In order to
       distinguish between those two situation it is advised to check the state
       variable value. If the state == IOTC_STATE_OK then the application has
       requested a disconnection via 'iotc_shutdown_connection'. If the state !=
       IOTC_STATE_OK then the connection has been closed from one side. */
    case IOTC_CONNECTION_STATE_CLOSED:
        
        /* When the connection is closed it's better to cancel some of previously
           registered activities. Using cancel function on handler will remove the
           handler from the timed queue which prevents the registered handle to be
           called when there is no connection. */
        if (IOTC_INVALID_TIMED_TASK_HANDLE != delayed_publish_task) {
            iotc_cancel_timed_task(delayed_publish_task);
            delayed_publish_task = IOTC_INVALID_TIMED_TASK_HANDLE;
        }

        if (state == IOTC_STATE_OK) {
            /* The connection has been closed intentionally. Therefore, stop
               the event processing loop as there's nothing left to do
               in this example. */
            iotc_events_stop();
        } else {
            ESP_LOGE(TAG, "connection closed - reason %d!", state);
            /* The disconnection was unforeseen.  Try reconnect to the server
            with previously set configuration, which has been provided
            to this callback in the conn_data structure. */
            iotc_connect(
                in_context_handle, conn_data->username, conn_data->password, conn_data->client_id,
                conn_data->connection_timeout, conn_data->keepalive_timeout,
                &on_connection_state_changed);
        }
        break;

    default:
        ESP_LOGE(TAG, "incorrect connection state value.");
        break;
    }
}



static void mqtt_task(void *pvParameters)
{
    /* Format the key type descriptors so the client understands
     which type of key is being represented. In this case, a PEM encoded
     byte array of a ES256 key. */
    iotc_crypto_key_data_t iotc_connect_private_key_data;
    iotc_connect_private_key_data.crypto_key_signature_algorithm = IOTC_CRYPTO_KEY_SIGNATURE_ALGORITHM_ES256;
    iotc_connect_private_key_data.crypto_key_union_type = IOTC_CRYPTO_KEY_UNION_TYPE_PEM;
    iotc_connect_private_key_data.crypto_key_union.key_pem.key = (char *) mqtt_values.mqtt_client_key;

    /* initialize iotc library and create a context to use to connect to the
    * GCP IoT Core Service. */
    const iotc_state_t error_init = iotc_initialize();

    if (IOTC_STATE_OK != error_init) {
        ESP_LOGE(TAG, " iotc failed to initialize, error: %d", error_init);
        vTaskDelete(NULL);
    }

    /*  Create a connection context. A context represents a Connection
        on a single socket, and can be used to publish and subscribe
        to numerous topics. */
    iotc_context = iotc_create_context();
    if (IOTC_INVALID_CONTEXT_HANDLE >= iotc_context) {
        ESP_LOGE(TAG, " iotc failed to create context, error: %d", -iotc_context);
        vTaskDelete(NULL);
    }

    /*  Queue a connection request to be completed asynchronously.
        The 'on_connection_state_changed' parameter is the name of the
        callback function after the connection request completes, and its
        implementation should handle both successful connections and
        unsuccessful connections as well as disconnections. */
    const uint16_t connection_timeout = 0;
    const uint16_t keepalive_timeout = 20;

    /* Generate the client authentication JWT, which will serve as the MQTT
     * password. */
    char jwt[IOTC_JWT_SIZE] = {0};
    size_t bytes_written = 0;
    iotc_state_t state = iotc_create_iotcore_jwt(
                             mqtt_values.aud,
                             /*jwt_expiration_period_sec=*/20*60, &iotc_connect_private_key_data, jwt,
                             IOTC_JWT_SIZE, &bytes_written);
                             printf("JWT: %s",jwt);
    
    if (IOTC_STATE_OK != state) {
        ESP_LOGE(TAG, "iotc_create_iotcore_jwt returned with error: %ul", state);
        vTaskDelete(NULL);
    }
   char *device_path= mqtt_values.mqtt_client_id;
   
    iotc_connect(iotc_context, "user", jwt, device_path, connection_timeout,
                 keepalive_timeout, &on_connection_state_changed);
                 ESP_LOGI(TAG,"connecting to GCP IOT at %s", device_path);
    
    iotc_events_process_blocking();

    iotc_delete_context(iotc_context);

    iotc_shutdown();

    vTaskDelete(NULL);
}




void start_mqtt_gcp(){
   
xTaskCreatePinnedToCore(mqtt_task,"qtt config",20000,NULL,4,NULL,1);

}
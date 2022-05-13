#include "mqtt_client.h"

#define star_struck 0
//#include "cJSON.h"
//wifi_app_message_e message_parse = PARSING_DONE; 
esp_mqtt_client_config_t config;
esp_mqtt_client_handle_t mqtt_handle;
typedef struct working_config{

   int dataFreq;
   long saveMinFreq;
   float minAmps;
   float minRPM;
}working_config_t;

typedef struct mqtt_config_params{
char aud[50];
char mqtt_host[100];
uint32_t mqtt_port;
char mqtt_client_id[100];
char mqtt_publish_link[100];
char mqtt_config_link[100];
char mqtt_endpoint[100];
char mqtt_publish_uri[100];
char mqtt_set_state_uri[100];
char mqtt_config_uri[100];
char mqtt_client_key[300];
unsigned char mqtt_cert[2500];
unsigned long long mqtt_project_id;

}mqtt_config_param_t;

working_config_t working_values;
mqtt_config_param_t mqtt_values;
void start_main_task(void);
//char* parse_data_string(cJSON *,char*);

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_netif.h"
#include "esp_log.h"

#include "esp_tls.h"
#include "esp_http_client.h"

#include "ArduinoJson.h"
extern "C" { 
        #include "main_task.h" 
        #include "wifi_app.h"
        }



static const char TAG[] = "MAIN_TASK";
StaticJsonDocument<3000> response;
void load_config(void *pvParameters){
                nvs_handle_t handle;
                esp_err_t error;
                error = nvs_open("storage",NVS_READWRITE,&handle);
                if(error == ESP_OK){
                        size_t size;
                        nvs_get_str(handle,"json_response",NULL,&size);
                         char *json_response = (char *)malloc(size+4);
                        error = nvs_get_str(handle,"json_response",json_response,&size);
                        switch(error){
                        case ESP_OK:
                           { ESP_LOGI(TAG,"json_response exists in nvs");
                            printf("%s \n",json_response);
                            DeserializationError err = deserializeJson(response, json_response);    
                           if(!err){

                                   working_values.dataFreq = response["dataFreq"];
                                   working_values.minAmps = response["minAmps"];
                                   working_values.minRPM = response["minRpm"];
                                   working_values.saveMinFreq = response["saveMinFreq"];
                                   printf("%u   %li   %2f   %2f  \n",working_values.dataFreq,working_values.saveMinFreq,working_values.minRPM,working_values.minAmps);
                                
                                strcpy(mqtt_values.aud,response["aud"]);
                                ESP_LOGI(TAG,"%s",mqtt_values.aud);
                                strcpy(mqtt_values.mqtt_host,response["mqttHost"]);
                                ESP_LOGI(TAG,"%s",mqtt_values.mqtt_host);
                                mqtt_values.mqtt_port= response["mqttPort"];
                                ESP_LOGI(TAG,"%u",mqtt_values.mqtt_port);
                                strcpy(mqtt_values.mqtt_client_id, response["mqttClientId"]);
                                ESP_LOGI(TAG,"%s",mqtt_values.mqtt_client_id);
                                strcpy(mqtt_values.mqtt_publish_link, response["mqttPublish"]);
                                ESP_LOGI(TAG,"%s",mqtt_values.mqtt_publish_link);
                                strcpy(mqtt_values.mqtt_config_link, response["mqttConfig"]);
                                ESP_LOGI(TAG,"%s",mqtt_values.mqtt_config_link);
                                strcpy(mqtt_values.mqtt_endpoint, response["endPoint"]);
                                ESP_LOGI(TAG,"%s",mqtt_values.mqtt_endpoint);
                                strcpy(mqtt_values.mqtt_publish_uri, response["publishURI"]);
                                ESP_LOGI(TAG,"%s",mqtt_values.mqtt_publish_uri);
                                strcpy(mqtt_values.mqtt_set_state_uri, response["setStateURI"]);
                                ESP_LOGI(TAG,"%s",mqtt_values.mqtt_set_state_uri);
                                strcpy(mqtt_values.mqtt_config_uri, response["configURI"]);
                                ESP_LOGI(TAG,"%s",mqtt_values.mqtt_config_uri);
                                strcpy(mqtt_values.mqtt_client_key, response["es"]);
                                ESP_LOGI(TAG,"%s",mqtt_values.mqtt_client_key);
                                strcpy((char *)mqtt_values.mqtt_cert, response["cert"]);
                                ESP_LOGI(TAG,"%s",mqtt_values.mqtt_cert);
                                mqtt_values.mqtt_project_id= response["id"];
                           } 
                           else{

                                   ESP_LOGI(TAG,"ERROR OCCURED IN PARSING !!!!");
                                   printf("%s",err.c_str());
                           }

                            
                           
                        }break;
                        default:
                                ESP_LOGI(TAG,"error loading json_response_from_nvs");
                                break;
                        }


                }
                else{
                        ESP_LOGI(TAG,"NVS LOADING FAILED!!!");
                }

wifi_app_send_message(PARSING_DONE);

vTaskDelete(NULL);
}

/*char* parse_data_string(cJSON *parse,char* key){
char * value;
cJSON *name= cJSON_GetObjectItemCaseSensitive(parse,key);
        if(cJSON_IsString(name)){
    
       memcpy(temp,name -> valuestring,sizeof(temp)); 
    }
    else {
        memcpy(temp,"0",sizeof("0"));
    }
    value = temp;
ESP_LOGI(TAG,"HELLO  %s",temp);
return value;

}*/

void start_main_task(void){
ESP_LOGI(TAG,"started main task");
//load_config();
xTaskCreatePinnedToCore(load_config,"load config",15000,NULL,4,NULL,1);



}
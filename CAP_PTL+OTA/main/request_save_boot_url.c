#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include "esp_mac.h"
#include "esp_http_client.h"
#include "request_save_boot_url.h"
#include "tasks_common.h"
#include "wifi_app.h"
#include "ota_config.h"

char DeviceId[18];
wifi_app_message_e msg=SAVED_BOOT_URL_TO_NVS;
static const char TAGI[] = "request_boot_URL";
extern const char postman_root_cert_pem_start[] asm("_binary_postman_root_cert_pem_start");
extern const char postman_root_cert_pem_end[]   asm("_binary_postman_root_cert_pem_end");
char global_response[MAX_HTTP_OUTPUT_BUFFER]={'/0'};
char buffer[500]="Bearer ";
const char auth_token[] = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE2NTA3ODE3MTgsImV4cCI6MTY1MDg2ODExOCwic3ViIjoiQUFCQkNDRERFRUZGIn0.IjhwzTIpt_aFK8qD0QsF3izjDsGV5rCrUGMfM7Y2C_QZ5jIDakxNSqiRWgoU7g7Gh6QvpQsJK_EFNWwtkGHkte6RPSB1ibnusywEnPzBIgMvscDrhZKYi4JCapIslxOu-tuloTrS2kPqKWF477UF_JLTinDv08u39EBtoikE4GHHDQU1cL4YaIy-pb5DCznWQ6o5Jg9KMb0VOO--2XD6MkGyWgt13M27X58Cw7O9yxAC0uuTYrqMHG9y8c6x8OQ-Cn__ITG4ZjO3WmFi1qjco77zKB4wze27SF4jdi-nE4RJj4yX4BRxl3xtzjsPoNT2SI0KmD4ufBrCHHCExKSKQA";
uint8_t g_recieve_flag = 0;
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAGI, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAGI, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAGI, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAGI, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAGI, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAGI, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAGI, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAGI, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAGI, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                if (output_buffer != NULL) {
                    free(output_buffer);
                    output_buffer = NULL;
                }
                output_len = 0;
                ESP_LOGI(TAGI, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAGI, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
    }
    return ESP_OK;
}



static void https_with_url(void)
{ 
	char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    esp_http_client_config_t config = {
        .url = "https://iot-door-boot-dot-smart-door-334013.uc.r.appspot.com/",
		///.path= "/get",
		.user_data = local_response_buffer,        // Pass address of local buffer to get response
		 . max_redirection_count = 5,
        .event_handler = _http_event_handler,
		.buffer_size_tx = 2048,
		.buffer_size = 3000,
		.cert_pem = postman_root_cert_pem_start,
		//.crt_bundle_attach = esp_crt_bundle_attach,

    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client,"Authorization",buffer);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_perform(client);
    //ESP_LOGI(TAGI,"%s",local_response_buffer);
    printf("%s\n",local_response_buffer);
    if (err == ESP_OK) {
        ESP_LOGI(TAGI, "HTTPS Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
        //if(strlen(local_response_buffer)>1000){g_recieve_flag =1;}
        //memcpy(global_response,local_response_buffer,strlen(local_response_buffer));
        //strcpy(dildar,local_response_buffer);
        if(strlen(local_response_buffer)>1000){
                nvs_handle_t my_handle;
                esp_err_t err;
                err = nvs_open("storage", NVS_READWRITE, &my_handle);
                if (err != ESP_OK) {
                        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
                    }
                else{
                            size_t size = sizeof(local_response_buffer);
                            err = nvs_get_str(my_handle, "json_response", NULL,&size);
                            switch(err){
                             case ESP_OK:
                                printf("boot config already in nvs");            
                             break;
                             case ESP_ERR_NVS_NOT_FOUND:
            	                printf("Updating json_response in NVS ... ");
            	                err = nvs_set_str(my_handle, "json_response", local_response_buffer);
            	                printf((err != ESP_OK) ? "Failed to set json_response!\n" : "Done\n");
                                err = nvs_commit(my_handle);
                                printf((err != ESP_OK) ? "Failed nvs commit!\n" : "done commit\n");
                                break;
                            default :
                                printf("Error (%s) reading!\n", esp_err_to_name(err));
                }

                }
                nvs_close(my_handle);
                g_recieve_flag =1; 
                }
                else{
                    g_recieve_flag = 0;
                ESP_LOGI(TAGI,"%s",local_response_buffer);
                }


        } else {
        ESP_LOGE(TAGI, "Error perform http request %s", esp_err_to_name(err));
        g_recieve_flag = 0;
    }
    vTaskDelay(10*portTICK_PERIOD_MS);
    //free(local_response_buffer);
    esp_http_client_cleanup(client);

}

static uint8_t check_parameter_exists(){
uint8_t exists = 0;
esp_err_t err;
nvs_handle_t nvs_handle;
err = nvs_open("storage",NVS_READWRITE,&nvs_handle);
if(err != ESP_OK){
    printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
}
else{
    size_t size = sizeof(global_response);
    err = nvs_get_str(nvs_handle,"json_response",&global_response,&size);
    switch(err){

        case ESP_OK:

            ESP_LOGI(TAGI,"json_response exists in nvs");
            ESP_LOGI(TAGI,"%s",global_response);
            exists =1;
            /*if(strlen(global_response)>1000){
                exists = 1;
                ESP_LOGI(TAGI,"json_response exists in nvs");
                ESP_LOGI(TAGI,"%s",global_response);
            }
            else{
                ESP_LOGI(TAGI,"REQUIRED PAYLOAD NOT RECIEVED TRY AGAIN TO REQUEST");
                exists =0;
            }*/
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGI(TAGI,"json_response does not exists in nvs..requesting server");
            exists = 0;
            break;
}
nvs_close(nvs_handle);
}
return exists;
}

void task_loop(void *pv_parameters){
for(;;){
uint8_t exists = check_parameter_exists();
if(exists==0  ){

    https_with_url();
    //check_parameter_exists();
    if(!g_recieve_flag){

        ESP_LOGI(TAGI,"HTTPS REQUEST FAILED TO GET DATA");
    vTaskDelay(1000*portTICK_PERIOD_MS);
    }
    https_with_url_for_ota(DeviceId);
}
else{
    break;
}

}
wifi_app_send_message(msg);
vTaskDelete(NULL);
}

void start_request_boot_url_task(void){
    
    nvs_handle_t jwt_key_handle;
    esp_err_t err;
    err = nvs_open("storage",NVS_READWRITE,&jwt_key_handle);
    if(err == ESP_OK){
            size_t size;
            nvs_get_str(jwt_key_handle,"JWT_KEY", NULL,&size);
            char *temp_jwt = malloc(size +  4);
            nvs_get_str(jwt_key_handle,"JWT_KEY",temp_jwt,&size);
            printf("%s",temp_jwt);
            if(size > 50){
            
            strcat(buffer,temp_jwt);
                
           
            }
            else{
                strcat(buffer,auth_token);
            }
        //free(temp_jwt);
            }
            nvs_close(jwt_key_handle);
             ESP_LOGI(TAGI,"LOADING NEW JWT: %s",buffer);
 xTaskCreate(task_loop, "task_loop", HTTP_REQUEST_BOOT_URL_TASK_STACK_SIZE, NULL,HTTP_REQUEST_BOOT_URL_TASK_PRIORITY, NULL);

}
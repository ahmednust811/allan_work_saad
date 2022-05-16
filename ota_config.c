#include "ota_config.h"
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include "esp_mac.h"
#include "esp_http_client.h"
#include "request_save_boot_url.h"

extern const char postman_root_cert_pem_start[] asm("_binary_postman_root_cert_pem_start");
extern const char postman_root_cert_pem_end[]   asm("_binary_postman_root_cert_pem_end");
char buffer_ota[500]="Bearer ";
char DeviceId[18];
char* TAGI = "OTA_TAG";
//const char auth_token[] = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE2NTA3ODE3MTgsImV4cCI6MTY1MDg2ODExOCwic3ViIjoiQUFCQkNDRERFRUZGIn0.IjhwzTIpt_aFK8qD0QsF3izjDsGV5rCrUGMfM7Y2C_QZ5jIDakxNSqiRWgoU7g7Gh6QvpQsJK_EFNWwtkGHkte6RPSB1ibnusywEnPzBIgMvscDrhZKYi4JCapIslxOu-tuloTrS2kPqKWF477UF_JLTinDv08u39EBtoikE4GHHDQU1cL4YaIy-pb5DCznWQ6o5Jg9KMb0VOO--2XD6MkGyWgt13M27X58Cw7O9yxAC0uuTYrqMHG9y8c6x8OQ-Cn__ITG4ZjO3WmFi1qjco77zKB4wze27SF4jdi-nE4RJj4yX4BRxl3xtzjsPoNT2SI0KmD4ufBrCHHCExKSKQA";
uint8_t g_recieve_flag_ota = 0;
// esp_http_client event handler
esp_err_t _http_ota_event_handler(esp_http_client_event_t *evt) {
    static char *output_buffer_ota;  // buffer_ota to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
	switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            break;
        case HTTP_EVENT_ON_CONNECTED:
            break;
        case HTTP_EVENT_HEADER_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;
        case HTTP_EVENT_ON_DATA:
           ESP_LOGD(TAGI, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer_ota is configured, copy the response into the buffer_ota
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer_ota == NULL) {
                        output_buffer_ota = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer_ota == NULL) {
                            ESP_LOGE(TAGI, "Failed to allocate memory for output buffer_ota");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer_ota + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAGI, "HTTP_EVENT_ON_FINISH");
            if (output_buffer_ota != NULL) {
                // Response is accumulated in output_buffer_ota. Uncomment the below line to print the accumulated response
                // ESP_LOG_buffer_ota_HEX(TAGI, output_buffer_ota, output_len);
                free(output_buffer_ota);
                output_buffer_ota = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAGI, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                if (output_buffer_ota != NULL) {
                    free(output_buffer_ota);
                    output_buffer_ota = NULL;
                }
                output_len = 0;
                ESP_LOGI(TAGI, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAGI, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
    }
    return ESP_OK;
}

void https_with_url_for_ota(char* deviceID)
{ 
	char localbuffer_otaStoreOTAurl[256];
	sprintf(localbuffer_otaStoreOTAurl,"%s%s",OTA_URL,deviceID);
	ESP_LOGI(TAGI, "Check OTA URL: %s",localbuffer_otaStoreOTAurl);
	char local_response_buffer_ota[MAX_HTTP_OUTPUT_BUFFER] = {0};
    esp_http_client_config_t config = {
        .url = localbuffer_otaStoreOTAurl,
		///.path= "/get",
		.user_data = local_response_buffer_ota,        // Pass address of local buffer_ota to get response
		 . max_redirection_count = 5,
        .event_handler = _http_ota_event_handler,
		.buffer_size_tx = 2048,
		.buffer_size = 3000,
		.cert_pem = postman_root_cert_pem_start,
		//.crt_bundle_attach = esp_crt_bundle_attach,

    };
	
	esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client,"Authorization",buffer_ota);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_perform(client);
    //ESP_LOGI(TAGI,"%s",local_response_buffer_ota);
    printf("%s\n",local_response_buffer_ota);
	
	if (err == ESP_OK) {
        ESP_LOGI(TAGI, "HTTPS Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
        //if(strlen(local_response_buffer_ota)>1000){g_recieve_flag_ota =1;}
        //memcpy(global_response,local_response_buffer_ota,strlen(local_response_buffer_ota));
        //strcpy(dildar,local_response_buffer_ota);
        if(strlen(local_response_buffer_ota)>1000){
                nvs_handle_t my_handle;
                esp_err_t err;
                err = nvs_open("storage", NVS_READWRITE, &my_handle);
                if (err != ESP_OK) {
                        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
                    }
                else{
                            size_t size = sizeof(local_response_buffer_ota);
                            err = nvs_get_str(my_handle, "json_response", NULL,&size);
                            switch(err){
                             case ESP_OK:
                                printf("boot config already in nvs");            
                             break;
                             case ESP_ERR_NVS_NOT_FOUND:
            	                printf("Updating json_response in NVS ... ");
            	                err = nvs_set_str(my_handle, "json_response", local_response_buffer_ota);
            	                printf((err != ESP_OK) ? "Failed to set json_response!\n" : "Done\n");
                                err = nvs_commit(my_handle);
                                printf((err != ESP_OK) ? "Failed nvs commit!\n" : "done commit\n");
                                break;
                            default :
                                printf("Error (%s) reading!\n", esp_err_to_name(err));
                }

                }
                nvs_close(my_handle);
                g_recieve_flag_ota =1; 
                }
                else{
                    g_recieve_flag_ota = 0;
                ESP_LOGI(TAGI,"%s",local_response_buffer_ota);
                }


        } else {
        ESP_LOGE(TAGI, "Error perform http request %s", esp_err_to_name(err));
        g_recieve_flag_ota = 0;
    }
    vTaskDelay(10*portTICK_PERIOD_MS);
    //free(local_response_buffer_ota);
    esp_http_client_cleanup(client);
}
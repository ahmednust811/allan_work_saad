#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <mbedtls/pk.h>
#include <mbedtls/error.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <esp_err.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sntp/sntp.h>
#include "wifi_app.h"
#include "esp_http_client.h"
//#include "passwords.h"
#include "base64url.h"
#include "ota_config.h"
#include "nvs.h"
//wifi_app_message_e msg_JWT =  JWT_ENCODING_SUCCESSFUL;
// This is an "xxd" file of the PEM of the private key.
//#include "device1_private_pem.h"
const char TAG[] = "JWT_MAIN";
//the global flag to control writing of jwt key and production of jwt process 
uint8_t g_write_flag = 0;
//extern const char alzoid_private_key_pem_start[] asm("_binary_alzoid_private_key_pem_start");
//extern const char alzoid_private_key_pem_end[] asm("_binary_alzoid_private_key_pem_end");
//extern const char alzoid_public_key_pem_start[] asm("_binary_alzoid_private_key_pem_start");
//extern const char alzoid_public_key_pem_end[] asm("_binary_alzoid_private_key_pem_end"); 
char alzoid_key_pem[] = "-----BEGIN PRIVATE KEY-----\n"\
"MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCcorF1jaJMuGzo\n"\
"N/XWwqkzBgTmTN9MA+tD+swcs8J8pkU7Vq11TL7+bLTaKDci6tYOpASyrWPO5QGA\n"\
"raCPtmpg2eN3ruiJjjQkRXe48G2xtWGKFqQz9WGcVgs8tGm+27F0sEqvmY5E4g6p\n"\
"AQ3ltL3bpvTtT50GKKyZZCKNvyZrI5wG0MVxFxkq+fgshCClatObFY8NBlnhkyCW\n"\
"CHnvnmyg8DfrmKQ3B4yh2HunEGIP+lhBrz4UYlAOxOtf0NUhnT43hK0KT6y3lnvM\n"\
"8Cco4uMken6gRVGHntuV1+vtFowiaqqR1/HIxgk8KHvagTOx+8WlPJPLodMf6LaZ\n"\
"ZrZIrhQhAgMBAAECggEBAJMRdwnm6EKczzTihiMVNwS/XA5YkziuXVHAw7EXXz7m\n"\
"Zw6l/curOZFtxAkpxUs1r7Kjcf3KK3JxNw34+E63ef5h1t/jDFdLe82qpBSzOmum\n"\
"4GRlidt3RCUj7P0ccEVRxbbIZ/ekeB5ZGma4pLHPGIyKoGHJZKgJvLVxfbgViB8D\n"\
"Ih7LxDdXS751o9m3j2rh7xFY1N63N3wH14XG1Tq24eeoS8+fLZT9d4ihcJkkInS6\n"\
"WduDjhovVrP2SvwWkFomt1WlqzH9/5vOfPE80Pkqt/rSSLm1gXTuv5awzC3DKF1o\n"\
"Bxa0UtCzrb0SZHwebPe11cK3ueOS2dZmKSXmeBPN5nkCgYEAzo6JL6BCHBsp6wwK\n"\
"Rk/8c/vZvS0cIBmAa4Cb2s12RKE71OKPioYe+/6LOMkJzffqemxPeePdhH4LtB3e\n"\
"POIyGXnoCGas4RQrfQNJLbm6RdKAZAH/tJldvqEeMOigbj4Kd3s949q6OuKqWGQI\n"\
"7W3a1pXFIkLAapyUKVpub7eWy3cCgYEAwiEQRADCvKcqwt4/FootmyzxOYa7/t/y\n"\
"m7t+ZgfwLDLROE8w1aT7DRr7NwZPnlEBPIoGcB6GCiF/c0XEx5bHyYl6qIScm0nD\n"\
"Vx7Qiw+9P7+mVYLyM9Kv9UCbmReNNtfeR5QyBh7xujosZhVuYHNT/cK4Lv9rymh0\n"\
"aciHEu2O0ycCgYBNsamCY4paXhFPWxEGT5HK4qGNGcmpfU4joZV+IKhquibyNWH+\n"\
"neJLsPwXGO96LNVixRjD5WTuMCcg0ddFGWP01zji6BQA+YA+Hp1I+MJK/xnSBDp1\n"\
"aPas/JeJmAa4UNfJB29JJH3GJnoik0YUYKze6CjlcSxFLa5BO8WseebbkwKBgHLy\n"\
"CuSJIsV7ogkyeV6KQMjSCp9KVQRsbAWkNgqKBr7U9CcIVN0by/8Ni9Qf64LNahMD\n"\
"kx5OJIXZIdMKa6LWd8OwoK0poGUefwiV2VuN3Tvi6cVWPL1Msh1gvQEf/eCMXzQ4\n"\
"3onbRrxC98kWRX3cq83kvxQs7xExWkTMlJYzjU8fAoGBAMgYEM74wkvwJj0LZD5o\n"\
"psptnUyOK9t9wmjuUWPew1P0DBra6gC9G8gkBD8P6oCvraXvLokHwhMVbNCRgm+K\n"\
"WQ217Nil2A2DoByW2YdKiTh/PAk1dHaYZJV/TWlNLYrb0LysNPJObssdt/HGrjGA\n"\
"c0jNr7cu6dMa0P4mAdP95CIH\n"\
"-----END PRIVATE KEY-----\n";

/*extern "C" {
    void app_main();
}*/
char DeviceId[18];
/**
 * Return a string representation of an mbedtls error code
 */
static char* mbedtlsError(int errnum) {
    static char buffer[200];
    mbedtls_strerror(errnum, buffer, sizeof(buffer));
    return buffer;
} // mbedtlsError


/**
 * Create a JWT token for GCP.
 * For full details, perform a Google search on JWT.  However, in summary, we build two strings.  One that represents the
 * header and one that represents the payload.  Both are JSON and are as described in the GCP and JWT documentation.  Next
 * we base64url encode both strings.  Note that is distinct from normal/simple base64 encoding.  Once we have a string for
 * the base64url encoding of both header and payload, we concatenate both strings together separated by a ".".   This resulting
 * string is then signed using RSASSA which basically produces an SHA256 message digest that is then signed.  The resulting
 * binary is then itself converted into base64url and concatenated with the previously built base64url combined header and
 * payload and that is our resulting JWT token.
 * @param projectId The GCP project.
 * @param privateKey The PEM or DER of the private key.
 * @param privateKeySize The size in bytes of the private key.
 * @returns A JWT token for transmission to GCP.
 */
char* createGCPJWT(const char* projectId, char* privateKey, size_t privateKeySize) {
    char base64Header[100];
    const char header[] = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
    base64url_encode(
        (unsigned char *)header,   // Data to encode.
        strlen(header),            // Length of data to encode.
        base64Header);             // Base64 encoded data.

    time_t now;
    time(&now);
    uint32_t iat = now;              // Set the time now.
    uint32_t exp = iat + 24*60*60;      // Set the expiry time.
    //nvs_handle_t handle_exp;
    //esp_err_t err;
    /*err = nvs_open("storage",NVS_READWRITE,&handle_exp);
    if(err == ESP_OK){
        uint32_t exp_old;
        err = nvs_get_u32(handle_exp,"expiration_time",&exp_old);
        switch(err){
                case(ESP_OK):
                        if((exp - exp_old)>24*60*60)
                    {
                        g_write_flag= 1;
                        nvs_set_u32(handle_exp,"expiration_time",exp);
                    }else{
                        ESP_LOGI(TAG,"TOKEN NOTE EXPIRED YET USING IT AGAIN");
                    g_write_flag = 0;
                    }
                    break;
                case ESP_ERR_NVS_NOT_FOUND:
                    ESP_LOGI(TAG,"TOKEN DOESNOT EXIST MAKING A NEW ONE");
                    nvs_set_u32(handle_exp,"expiration_time",exp);
                    g_write_flag = 1;
                    break;
        }
        nvs_commit(handle_exp);
        nvs_close(handle_exp);
    }
    if(g_write_flag==0){
        return NULL;
    }*/
    char payload[100];
    sprintf(payload, "{\"iat\":%d,\"exp\":%d,\"sub\":\"%s\"}", iat, exp, projectId);

    char base64Payload[100];
    base64url_encode(
        (unsigned char *)payload,  // Data to encode.
        strlen(payload),           // Length of data to encode.
        base64Payload);            // Base64 encoded data.

    uint8_t headerAndPayload[800];
    sprintf((char*)headerAndPayload, "%s.%s", base64Header, base64Payload);

    // At this point we have created the header and payload parts, converted both to base64 and concatenated them
    // together as a single string.  Now we need to sign them using RSASSA

    mbedtls_pk_context pk_context;
    mbedtls_pk_init(&pk_context);
    int rc = mbedtls_pk_parse_key(&pk_context, (unsigned char *)privateKey, privateKeySize, NULL, 0);
    if (rc != 0) {
        printf("Failed to mbedtls_pk_parse_key: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
        return NULL;
    }

    uint8_t oBuf[5000];

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    const char* pers="MyEntropy";
                
    mbedtls_ctr_drbg_seed(
        &ctr_drbg,
        mbedtls_entropy_func,
        &entropy,
        (const unsigned char*)pers,
        strlen(pers));
    

    uint8_t digest[32];
    rc = mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), headerAndPayload, strlen((char*)headerAndPayload), digest);
    if (rc != 0) {
        printf("Failed to mbedtls_md: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
        return NULL;        
    }

    size_t retSize;
    rc = mbedtls_pk_sign(&pk_context, MBEDTLS_MD_SHA256, digest, sizeof(digest), oBuf, &retSize, mbedtls_ctr_drbg_random, &ctr_drbg);
    if (rc != 0) {
        printf("Failed to mbedtls_pk_sign: %d (-0x%x): %s\n", rc, -rc, mbedtlsError(rc));
        return NULL;        
    }


    char base64Signature[600];
    base64url_encode((unsigned char *)oBuf, retSize, base64Signature);

    char* retData = (char*)malloc(strlen((char*)headerAndPayload) + 1 + strlen((char*)base64Signature) + 1);

    sprintf(retData, "%s.%s", headerAndPayload, base64Signature);

    mbedtls_pk_free(&pk_context);
    return retData;
}

void run(void *pvParameter) {
    printf("Task starting!\n");
    uint8_t mac[6];
    esp_read_mac(&mac,ESP_MAC_WIFI_STA);
    char mac_adress[18];
    sprintf(mac_adress,"%02X%02X%02X%02X%02X%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    const char* projectId = mac_adress;
    printf("%s",projectId);
    sprintf(DeviceId,"%02X%02X%02X%02X%02X%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    ESP_LOGI(TAG,"WAITING FOR SNTP");
    ESP_LOGI(TAG,"OTA URL: %s%s",OTA_URL,DeviceId);
    
    
    sntp_setservername(0, "time.google.com");
    //sntp_setservername(1,"time.nist.gov");
    sntp_init();
    // https://www.epochconverter.com/
    time_t now = 0;
    time(&now);
    while(now < 5000) {
        vTaskDelay(1000 * portTICK_RATE_MS);
        time(&now);
    }
    size_t size = sizeof(alzoid_key_pem);
    char* jwt = createGCPJWT(projectId,alzoid_key_pem, size);
    if(jwt != NULL){
    nvs_handle_t handle;
    esp_err_t err;
    err = nvs_open("storage",NVS_READWRITE,&handle);
    if(err != ESP_OK){
    printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else{
        size_t size;
    err= nvs_get_str(handle,"JWT_KEY",NULL,&size);
    switch(err){
        case ESP_OK:  
            printf("already in nvs");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("saving key in nvs");
            err = nvs_set_str(handle, "JWT_KEY", jwt);
            printf((err != ESP_OK) ? "Failed to set json_response!\n" : "Done\n");
            err = nvs_commit(handle);
            printf((err != ESP_OK) ? "Failed nvs commit!\n" : "done commit\n");
            
            break;
        default:
        printf("unknow error");
        break; 
    }
    }
    nvs_close(handle);
                printf("JWT: %s\n", jwt);
            free(jwt);
        }   
        wifi_app_send_message(JWT_ENCODING_SUCCESSFUL);
        
        vTaskDelete(NULL);
    }

void start_jwt_encoding_task(){
    xTaskCreatePinnedToCore(&run,"run",16000,NULL,2,NULL,1);
}


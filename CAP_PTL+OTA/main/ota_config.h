
#ifndef JWT_OTA_CONFIG_H
#define JWT_OTA_CONFIG_H

#include "esp_system.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

#define OTA_URL "https://update-service-dot-smart-door-334013.uc.r.appspot.com/"


extern char DeviceId[18];

void https_with_url_for_ota(char* deviceID);
#endif
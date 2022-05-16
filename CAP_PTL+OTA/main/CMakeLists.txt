# Edit following two lines to set component requirements (see docs)
idf_component_register(SRCS main.c rgb_led.c wifi_app.c http_server.c app_nvs.c wifi_reset_button.c dns_server.c request_save_boot_url.c JWT_MAIN.c base64url.c main_task.cpp GCP_MQTT.c "ota_config.c"
						INCLUDE_DIRS "." "src"  
						EMBED_FILES webpage/app.css webpage/app.js webpage/favicon.ico webpage/index.html webpage/jquery-3.3.1.min.js webpage/postman_root_cert.pem webpage/alzoid_private_key.pem) 
# "iot-device-sdk-embedded-c/examples/common/src"
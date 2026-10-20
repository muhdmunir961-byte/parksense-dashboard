#pragma once
// Copy to cloud_config.h, then set CLOUD_UPLINK_ENABLED to true.
// ESP-NOW pods in the original project use 2.4 GHz Wi-Fi channel 1.
// Your router MUST use 2.4 GHz channel 1 for this sample setup.
#define CLOUD_UPLINK_ENABLED false
#define CLOUD_WIFI_SSID "YOUR_2_4GHZ_WIFI_SSID"
#define CLOUD_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define CLOUD_API_URL "https://parksense.example.com/api/device/telemetry"
#define CLOUD_DEVICE_TOKEN "PASTE_SAME_TOKEN_AS_COOLIFY_ENV_DEVICE_TOKEN"

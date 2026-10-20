#pragma once
// COPY TO cloud_config.h OR run: python tools/setup_firmware.py
// ESP32 and all four ESP-NOW sensor pods use channel 1.
// Before enabling, CONFIRM the hotspot is 2.4GHz and on CHANNEL 1.
// Do not push your filled-in cloud_config.h to GitHub.

#define CLOUD_UPLINK_ENABLED false
#define CLOUD_WIFI_SSID "YOUR_ANDROID_HOTSPOT_SSID"
#define CLOUD_WIFI_PASSWORD "YOUR_ANDROID_HOTSPOT_PASSWORD"
#define CLOUD_API_URL "https://parkingdashboard.my.id/api/device/telemetry"
#define CLOUD_DEVICE_TOKEN "COPY_THE_24PLUS_CHARACTER_COOLIFY_DEVICE_TOKEN"
#define PARKSENSE_AP_PASSWORD "CHOOSE_ANOTHER_8PLUS_CHARACTER_PASSWORD"

#!/usr/bin/env python3
"""Legacy setup shim: ParkSense now needs edits to main.cpp only."""
from pathlib import Path
import sys

MAIN_CPP = Path(__file__).resolve().parent.parent / "firmware" / "esp32-main" / "src" / "main.cpp"
print("ParkSense now uses ONE FILE for ESP32 setup.")
print("Open:", MAIN_CPP)
print("At the TOP, replace just the 3 ISI_... placeholders:")
print("  CLOUD_WIFI_SSID, CLOUD_WIFI_PASSWORD, CLOUD_DEVICE_TOKEN")
print("CLOUD_UPLINK_ENABLED is already true and the API URL is already filled.")
print("Check that your Wi-Fi hotspot/router is 2.4 GHz CHANNEL 1 before flashing.")
print("Save main.cpp locally, connect MAIN ESP32 USB, then PlatformIO Upload.")
print("Do NOT upload real secrets to GitHub. No files were changed.")
sys.exit(0)

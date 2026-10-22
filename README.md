# ParkSense — ready-to-connect FYP dashboard

**Goal:** Deploy the website once, configure ESP32 once, and adjust its **speed warning threshold** from the tablet or rotary encoder. Physical LEDs and buzzer still run on ESP32, independently of Coolify. No extra app, account or 3D installation on the tablet.

**Dashboard:** https://parkingdashboard.my.id

## IMPORTANT — Phone hotspot update from your local Cline main.cpp (October 2026)

The **Coolify server + dashboard now accept the optional `wifiChannel` telemetry field** from your **locally edited** main ESP32 firmware. The web shows CH 1–14 and number of pods with recent readings; it warns when the ESP32 is receiving cloud telemetry but some/all pods are offline. Old firmware without `wifiChannel` still works and displays **CHANNEL — / UNVERIFIED**.

**Two distinct ESP32 main builds exist:**
- **Repository `firmware/esp32-main/src/main.cpp`** is currently the *older channel-1-restricted* build (WiFi.begin SSID, password, 1); use channel-1 router/hotspot if you flash it.
- **Your updated Cline `c:\\coding\\ParkSense\\src\\main.cpp`**, supplied in chat, uses `WiFi.begin(..., 0)`, changes main ESP32 AP to follow the 2.4 GHz hotspot, and sends `wifiChannel`. This updated firmware has **not** been uploaded to this repo, because the shared copy contained live Wi-Fi/AP passwords and a device token. **Do not flash the repo copy over your updated local Cline firmware.**

**Critical limitation:** ESP-NOW pods flashed with `esp_wifi_set_channel(1,...)` are still locked to CH 1. Setting `peerInfo.channel=0` only uses a sender's current radio channel; **it does not scan, retune or auto-pair with another ESP32 channel**. If hotspot uses CH 6, verify each pod is configured for CH 6 through controlled testing, or choose a fixed CH 1 router. A channel change on the main can disrupt pods until their own channel configuration is updated. Website reports the channel received from the main and active data per pod, **not radio channel proof**.

**Security:** your firmware paste exposed the Wi-Fi password, ESP32 local AP password and device token in a chat. Change all three locally (and `DEVICE_TOKEN` in Coolify) before use. Never paste actual credentials or live firmware with embedded secrets into public GitHub; keep local copies/private secure storage.



## Your remaining steps (main ESP32 + Android hotspot)

### 0. Before flashing: keep the original firmware and verify radio channel
- Back up your currently working `main.cpp`, your ESP32 board type, pin connections and serial-port settings. **Never overwrite a proven working build without a restore copy.**
- Main + four ESP-NOW sensor pods are all fixed to **2.4 GHz Wi-Fi channel 1**.
- Set the Android hotspot band to **2.4 GHz**, then *check the hotspot's actual channel with a Wi-Fi analyzer*. Many phones automatically choose **channel 6 or 11**, even with 2.4 GHz selected.
- **If the phone hotspot is not channel 1, STOP:** do not enable cloud Wi-Fi/flash the cloud-enabled firmware. The dashboard can run in demo mode or the original local ESP32 dashboard can still be used. A router with fixed 2.4 GHz channel 1 is a fallback.
- This project displays telemetry and warns; it **does not control physical vehicle braking, accelerator or throttle**.

### 1. Set up the Coolify server (once)
- Use this repository as Coolify Application (Dockerfile build; container port `3000`).
- Domain: `https://parkingdashboard.my.id` and automatic HTTPS.
- Environment variables (use *different*, long, random values):
  ```env
  PORT=3000
  DASHBOARD_PASSWORD=unique_login_password_at_least_16_chars
  DEVICE_TOKEN=a_different_random_token_at_least_24_chars
  ```
- Deploy. Verify `https://parkingdashboard.my.id/health` returns `{"status":"ok"}`, then sign into the website with `DASHBOARD_PASSWORD`.
- **Never paste these secrets into GitHub, an issue, a screenshot, a chat or a public log.** Password is for web login, token only for the ESP32 device.

### 2. Set only three values in `main.cpp` (no wizard / config file)
Download or clone this repository to your **own laptop**. Open `firmware/esp32-main/src/main.cpp`, then fill **only three placeholders** at the top, with cloud already enabled:

```cpp
#define CLOUD_UPLINK_ENABLED true
#define CLOUD_WIFI_SSID "YOUR_CONFIRMED_2_4_GHZ_CHANNEL_1_WIFI_NAME"
#define CLOUD_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define CLOUD_DEVICE_TOKEN "YOUR_COOLIFY_DEVICE_TOKEN_24_PLUS_CHARACTERS"
#define CLOUD_API_URL "https://parkingdashboard.my.id/api/device/telemetry"
```

The **actual repo contains obvious `ISI_...` placeholders**. Replace those texts in your local copy. Never push `main.cpp` containing real Wi-Fi credentials/token to GitHub or paste them into a chat. The local ESP32 hotspot password is `PARKSENSE_AP_PASSWORD`, currently `123456789`; change it to a unique string with at least 8 characters for non-lab use.

Until all three fields are filled and pass validation, the ESP32 does **not** start cloud Wi-Fi, but its local ESP-NOW/LED/buzzer logic is still available. The old `cloud_config.h` wizard is no longer needed or read by `main.cpp`. Use a **verified channel 1** hotspot/router (not merely 2.4 GHz).

### 3. Compile + flash *main ESP32 only*
Connect your **main ESP32 via USB** with the proper USB data cable. Have PlatformIO installed in VS Code or code-server. Use the project that contains `firmware/esp32-main/platformio.ini`.

```bash
pio run -d firmware/esp32-main -t upload
```

The 4 ESP-NOW pod firmwares should **not** be reflashed unless their packet struct / ID / ESP-NOW channel are known to differ; preserve working pod firmware. If PlatformIO fails, inspect the compiler error rather than guessing / changing physical wires.

### 4. Verify everything after upload
- Open serial monitor at **115200 baud** on the main ESP32 and check ESP-NOW starts, cloud connects **on channel 1**, and there are no HTTP/TLS errors.
- **Prove all four pods** F / R / L / Right report live values. A missing pod must show **OFFLINE**.
- Check the GPIO14 switch, rotary encoder speed limit, GPS freshness and the actual LED/buzzer safety output at distances **<15cm**, **15–34cm** and **35–54cm** with stationary test objects. In **Parking Assist**, change the speed alert threshold with `+`, `−`, reset to 15 or enter a value (5–100 km/h) and click **Apply**. Wait for `ESP32 CONFIRMED` before claiming success.
- Tablet connects to internet (same phone hotspot works). Open `https://parkingdashboard.my.id`, log in, then wait for device telemetry. A healthy WSS connection by itself does **not** mean the ESP32 is online.
- If you see offline, check hotspot **channel 1**, SSID/password, DEVICE_TOKEN, TLS trust chain, phone data coverage and ESP32 serial output. Some Cloudflare / reverse-proxy TLS certificates chain to a root other than included `ISRG Root X1`; if so, install the **actual trusted CA certificate** in `cloud_ca.h`. Never switch to `setInsecure()`.

## Set speed-warning threshold directly from the dashboard
Open **Parking Assist** → **TARGET ALERT LIMIT**, choose a whole number from **5 to 100 km/h**, then click **Apply**. The cloud-connected main ESP32 receives the command in the reply to its next authenticated HTTPS telemetry POST. The dashboard shows **Pending** until the ESP32 acknowledges the new limit on a subsequent telemetry reading. You can also use the + / - buttons and Reset (15 km/h).

This controls **only the warning threshold**, not the actual vehicle speed, throttle, motor or brakes. The physical rotary encoder continues to work and can override the warning threshold; the GPIO14 hardware warning switch is unchanged. Do not assume the command worked until the dashboard says **ESP32 CONFIRMED**. Control requires the **latest firmware flashed to the main ESP32**, Coolify device connected to channel-1 hotspot, and a logged-in dashboard session. Demo mode is simulation-only. Commands expire if the ESP32 doesn't acknowledge promptly. Direct commands to browser WebSocket are not accepted.

## What is already built
- **Coolify Node.js server:** authenticated HTTPS device POST, read-only WebSocket telemetry relay, login and session cookies, health/status endpoint; same-origin, session-protected POST for setting the ESP32 speed **warning threshold only**, with explicit device acknowledgement.
- **Tablet-friendly four-page website:** Overview, Parking Assist, Diagnostics, Settings. Static demo scenarios allow display testing with no hardware.
- **Real 3D gallery:** seven creator-hosted CC BY references: Perodua Myvi (**generation unverified**), Proton Saga 1985 lightweight and HD, Proton X50, Proton Waja, Proton Jumbuck and Honda Civic FE police car (2022). Unverified years/trims are labelled; larger meshes can load slower on tablets. Sketchfab is an external embed with creator credits, not OEM CAD. Parking Assist still uses a separate *schematic* for four sensor locations.
- **Main firmware:** existing ESP32 physical warning logic, rotary encoder and local dashboard `http://192.168.4.1`; optional HTTPS uplink runs in a separate FreeRTOS task and retries connecting to **channel 1** every 30 seconds. It receives only validated `set_speed_limit` commands (5–100 km/h), updates the warning threshold in RAM and acknowledges on subsequent telemetry. A physical encoder change can still override the setting. **Cloud upload failure must never be treated as hardware-safe confirmation.**
- **Automated checks:** `npm test` for authentication/telemetry/UI and static firmware safeguards. GitHub Actions `ParkSense quality gate` also compiles PlatformIO firmware with cloud ON/OFF (check green results before flashing).

## Main board pins (confirm against your original wiring)

| Function | Pin |
| --- | --- |
| Red / Yellow / Green LEDs | 18 / 19 / 21 |
| Buzzer with driver/transistor | 13 |
| GPIO-to-GND parking-enable switch | 14 |
| Rotary encoder CLK / DT / SW | 25 / 26 / 27 |
| GPS UART RX / TX | 16 / 17 |

**Safety thresholds (distance/PIR fixed in firmware; GPS speed warning limit can be changed from tablet or rotary encoder):** distance <15cm solid RED + continuous buzzer; 15–34cm RED + short pulses; 35–54cm YELLOW + slow beeps; overspeed based on GPS and the rotary-encoder limit; PIR movement triggers alerts; local GPIO14 switch disables physical warnings. This is a **prototype warning system**, not a safety-certified speed limiter. Park and test on stationary objects.

## Data flow and simple diagnosis

```text
4 sensor pods --ESP-NOW CH1--> main ESP32 --HTTPS POST/1s--> Coolify
                                   |                        |
                                   | Wi-Fi AP               | secure WSS after login
                                   v                        v
                        192.168.4.1 (local)       Tablet / Dashboard
```

Device POST URL: `https://parkingdashboard.my.id/api/device/telemetry` (bearer `DEVICE_TOKEN`).

### Remote speed-warning-limit control (tablet → ESP32)
1. In **Parking Assist**, use **+ / −**, **reset 15**, or enter **5–100 km/h** and press **Apply**. Browser demo mode changes simulated values only.
2. Only a logged-in user on the **same-origin** website can POST `{"value":25}` to `/api/control/speed-limit`. The API rejects offline ESP32s, firmware without explicit support, invalid values and extra unacknowledged commands.
3. Coolify queues one 12-second command in RAM (not persistent). ESP32 receives it in its next authenticated HTTPS telemetry *response* (`{"command":{"type":"set_speed_limit","id":"...","value":25}}`).
4. ESP32 accepts **only** speed warning limit 5–100 km/h; changes **only** `speedLimit` (used in LED/buzzer overspeed warning). Next HTTPS POST returns `commandApplied` ID and actual `speedLimit`; backend confirms only when both match.
5. Dashboard shows **pending**, **ESP32 CONFIRMED**, or **failed**. If the phone/hotspot drops, the command expires rather than executing much later. No physical actuator control, no throttle/brake connections or autonomous speed regulation is implemented.
6. Rotary encoder continues to work locally even without internet; manual adjustment may override the last cloud setting. Neither option disables the hardware alarm switch or bypasses proximity/PIR warnings.

**Security:** app login (16+ password), device bearer token (24+), same-origin POST, strict JSON/integer/range validation, 12-second command expiry, one outstanding command, HTTPS certificate verification; commands cannot be sent over unauthenticated WebSocket. This is an **alert limit**, not a vehicle speed limiter.
Expected JSON fields: `maxSpeed, speedLimit, systemEnabled, distF, distR, distL, distRefR, humanF, humanR, humanL, humanRefR`; offline distances are `-1`. If GPS has no fresh fix, speed may read 0 **without proving the vehicle is stationary**. Coolify reports device offline after ~5 seconds without telemetry; no long-term data logging is included.

**Build/check locally:** `npm install && npm test` for server and UI; `pio run -d firmware/esp32-main` for firmware (PlatformIO required).

Only `firmware/esp32-main/src/main.cpp` is edited locally. Its full source remains part of the PlatformIO project, and the HTTPS certificate validation is embedded in it. The legacy `cloud_config.example.h` is no longer used. Keep real credentials in a private local copy of `main.cpp` and never commit them. A valid HTTPS/TLS handshake is required; no insecure fallback.

**Deployment notes:** use GitHub Actions `Deploy ParkSense to Coolify` for a manual redeploy only when its separate `COOLIFY_API_TOKEN` secret and Coolify HTTPS URL are configured. Otherwise use the Coolify **Deploy** button. Code pushed to GitHub **does not prove deployment completed**.

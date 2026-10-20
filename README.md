# ParkSense — Coolify Dashboard (Refined UI) + ESP32 firmware

**Updated to match the latest `main.cpp` provided in chat, NOT the earlier prototype wiring/alert logic.**
The main ESP32 still serves its local read-only dashboard at `http://192.168.4.1` and also sends live data by HTTPS to a Coolify-hosted dashboard when cloud uplink is enabled. The Coolify server relays authenticated sensor readings to logged-in viewers via WSS.

## Refined interface (visual redesign only)

The UI is now a restrained **light industrial/OEM diagnostic dashboard**: off-white surfaces, graphite typography, real warning colors only when appropriate, clear sensor readouts, mobile layout and consistent password login. This is not a rendered mockup: `public/index.html` and `public/login.html` are the actual pages served by Coolify.

- **Overview:** GPS-derived vehicle speed, configured warning threshold, ESP32 device/parking status.
- **Surround view:** real F/R/L/right values with a static top-down vehicle diagram; online/offline states, PIR and proximity warnings.
- **Sensor diagnostics:** separate, responsive four-pod list with clear distance/status values.
- **Demo scenarios:** Normal, Caution (43 cm), Critical (11 cm) and Overspeed (27 km/h). These simulate readouts **only**; they never command the ESP32, LED, or buzzer.
- **Live mode:** retains the same telemetry JSON fields, WebSocket relay and read-only hardware operation.
- **Authentication:** login form restyled; access policy is unchanged.

The full `public/index.html` can be opened locally for a no-hardware interactive demo. Screenshot previews are included in the archive. For real connections, deploy the app with Coolify.

## Deploy to Coolify

1. Put the ZIP contents in a Git repository (the repo root must contain `Dockerfile`, `server.js`, `package.json`, `public/`).
2. Coolify → New Resource → Application → Git repo → build pack **Dockerfile**. Exposed container port **3000**.
3. Add domain e.g. `https://parksense.example.com`, correctly configured for your server in DNS.
4. Add Coolify environment variables:

   ```env
   PORT=3000
   DASHBOARD_PASSWORD=<a unique random password, at least 16 characters>
   DEVICE_TOKEN=<a DIFFERENT random token, at least 24 characters>
   ```

5. Deploy, check `https://YOUR_DOMAIN/health`, then visit the domain to sign in.

**IMPORTANT:** The backend intentionally rejects tokens with fewer than 24 characters. A homemade 5-digit token **will not work and would be insecure** on the public Internet. Generate with `openssl rand -hex 32`; NEVER put secrets in GitHub or screenshots. `DASHBOARD_PASSWORD` is for website login and `DEVICE_TOKEN` is ONLY for ESP32's authenticated POST.

## ESP32 firmware / cloud setup

1. Open `firmware/esp32-main/` with PlatformIO, create or edit `include/cloud_config.h` from `cloud_config.example.h`.
2. Change `CLOUD_UPLINK_ENABLED` to `true`, insert your **2.4 GHz** router Wi-Fi SSID/password, `https://YOUR_DOMAIN/api/device/telemetry`, and the **same** strong `DEVICE_TOKEN` as on Coolify.
3. The main firmware's ESP-NOW channel is **1**. Configure the Wi-Fi router 2.4 GHz radio to **fixed channel 1**, not Auto. The ESP-NOW pods and the Wi-Fi station must use the same channel; this is a documented ESP-NOW requirement. If router is on a different channel, the firmware disconnects that connection rather than silently breaking the pods.
4. Firmware uses HTTPS certificate validation with `cloud_ca.h` (ISRG Root X1). If your domain is proxied through Cloudflare using a certificate anchored to a different CA, supply the correct trusted root CA or use a compatible valid Let's Encrypt chain; do **not** disable verification. NTP time sync is required for validating certificates.
5. Flash the ESP32 main board. Flash sensor pods IDs 0,1,2,3 (front, rear, left, right). Verify with Serial Monitor 115200 baud. Note: the `esp32-sensor` sample in this package follows the previously provided `sensor.cpp`; if your actual sensor firmware has changed, compare the packet struct and channel before flashing.
6. Local dashboard: connect to ESP32 Wi-Fi `Monitor Dashboard` (original password `123456789`, change it for non-lab use), then visit `http://192.168.4.1`. Cloud dashboard: visit `https://YOUR_DOMAIN` from any Internet-connected browser after logging in.

In live mode the dashboard is **read-only** by design: turning the encoder sets the speed alert threshold (5–100 km/h); pushing the encoder resets it to 15 km/h. Demo-mode `+/-` buttons only change simulated readings. Web commands do **not** control braking, engine throttle, real speed, physical LED/buzzer, or the latching parking switch.

## Correct hardware pins (latest main.cpp)

| Purpose | ESP32 GPIO |
| --- | --- |
| Red LED | **18** |
| Yellow LED | **19** |
| Green LED | **21** |
| Buzzer output | **13** (use an appropriate external transistor/driver for a 5V buzzer) |
| Latching power/parking switch | **14** (`GPIO14` short to GND = alerts ON; open = all alerts OFF) |
| Rotary CLK / DT / SW | **25 / 26 / 27** |
| GPS UART RX / TX | **16 / 17** |

LED output defaults to active HIGH. GPIO14 is a **software alert-enable switch**, not a hardware power cut-off. ESP32 outputs are not 5V tolerant.

## Alert logic from the user-supplied firmware

| Condition (parking switch must be ON) | Physical warning |
| --- | --- |
| Valid online pod distance **<15 cm** | Solid RED and continuous buzzer |
| Recent GPS vehicle speed **> speed limit** | Blinking RED and repeated beeps |
| Valid distance **15–34 cm** | Fast RED and intermittent buzzer |
| Valid distance **35–54 cm** | Solid YELLOW and slow beep |
| Fresh PIR movement | Two short buzzer beeps with a 1-second gap, queued behind other warnings |
| No hazard, pod online | GREEN LED |
| All pods offline | No false GREEN indicator |
| Parking switch OFF | All alert LEDs and buzzer OFF |

The UI reads ESP-NOW sensor packets only if they arrived in the last ~2 seconds and shows **OFFLINE (-1)** otherwise. The speed shown is **GPS-only**. If there is no recent valid GPS fix (>3 seconds stale), the main firmware reports 0 km/h; this does not establish that the vehicle is stationary.

## Transport and data schema

ESP32 main → `POST https://YOUR_DOMAIN/api/device/telemetry` with `Authorization: Bearer <DEVICE_TOKEN>` and JSON:

```json
{
  "maxSpeed": 8, "speedLimit": 15, "systemEnabled": true,
  "distF": 85, "humanF": 0,
  "distR": 120, "humanR": 0,
  "distL": 64, "humanL": 0,
  "distRefR": 95, "humanRefR": 0
}
```

Coolify server validates required fields and relays telemetry over a login-protected `wss://YOUR_DOMAIN/ws` socket. No direct WebSocket link from public HTTPS to `ws://192.168.4.1/ws` is required or supported. The server reports the ESP32 offline after 5 seconds with no uploads. No permanent database/history is included.

**Limitations:** Uplink currently only starts if the main board connects to valid Wi-Fi at startup. The firmware doesn't retry a failed initial association automatically, and SSL trust may need adjustment for your actual certificate. If the cloud fails, hardware warning logic and the local ESP32 dashboard still operate. DO NOT consider this a vehicle safety-certified speed limiter; it monitors and warns but does not physically limit speed.

## Files

- `public/index.html`: responsive live/dashboard demo; `public/login.html`: password login.
- `server.js`: Node.js HTTP + session authentication, device token validator, secure WebSocket relay.
- `Dockerfile`, `.env.example`, `package.json`: Coolify.
- `firmware/esp32-main/src/main.cpp`: **updated** main controller based on the latest user source.
- `firmware/esp32-main/include/cloud_config.example.h`: copy to **untracked** `cloud_config.h`, then set your own credentials.
- `firmware/esp32-sensor/`: sensor pod example from earlier source (verify wiring).

## Tests

`npm install && npm test` in the project folder (requires Internet access for `ws`). The JS code has been syntax checked. C++ firmware compilation and physical hardware operation must still be tested using PlatformIO on your machine.


## Interface redesign (Flyme Auto / Proton e.MAS inspired)

ParkSense uses its own brand and artwork. The visual style adapts the minimalist, light vehicle controls layout found on Proton e.MAS infotainment systems, with top-down vehicle overview, four readable sensor indicators, simplified vehicle status and touch-friendly bottom navigation. It is *not* official Proton software or a feature-for-feature replica.

The app still uses the unchanged telemetry schema and Coolify HTTPS relay. Physical parking alarms, GPS validity, ESP-NOW sensor channels and the GPIO14 switch remain governed by the ESP32 firmware, not by web visuals. In the demo, limits and scenario buttons affect only simulated values.

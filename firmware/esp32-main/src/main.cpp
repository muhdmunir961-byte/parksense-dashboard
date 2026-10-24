#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <esp_now.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <esp_wifi.h>
#include <esp_arduino_version.h>
// ============================================================================
// PARKSENSE / MAIN ESP32 — HOTSPOT AUTO CHANNEL / 3 MAKLUMAT SAHAJA
// ============================================================================
// 1. Hotspot 2.4 GHz boleh guna mana-mana channel; SENSOR POD MESTI ikut
//    channel yang sama. Sensor pod CH 1 TAK ikut hotspot automatik.
// 2. Di laptop sendiri, GANTI tulisan ISI_... dalam 3 baris di bawah.
// 3. Simpan dan upload MAIN ESP32 melalui PlatformIO. Tiada fail lain diedit.
// Jangan push semula main.cpp yang sudah mengandungi password / token ke GitHub.
//
// Cloud sentiasa disediakan, tetapi TIDAK akan cuba connect selagi maklumat
// masih kosong / placeholder. LED, buzzer dan empat pod kekal berfungsi lokal.
#define CLOUD_UPLINK_ENABLED true
#define CLOUD_WIFI_SSID "ISI_NAMA_HOTSPOT_WIFI_2_4GHZ"
#define CLOUD_WIFI_PASSWORD "ISI_PASSWORD_WIFI"
#define CLOUD_DEVICE_TOKEN "ISI_DEVICE_TOKEN_COOLIFY_MIN_24"
#define CLOUD_API_URL "https://parkingdashboard.my.id/api/device/telemetry"

// Pilihan tambahan: ubah password Wi-Fi lokal "Monitor Dashboard" kalau mahu.
// Password lama dikekalkan untuk keserasian; sebaiknya tukar ke 8+ aksara unik.
#define PARKSENSE_AP_PASSWORD "123456789"


#include <cstring>
#if CLOUD_UPLINK_ENABLED
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebSocketsClient.h>
#include <time.h>
// HTTPS TLS trust anchor is embedded in main.cpp to simplify one-file setup.
// Keep certificate validation ENABLED; never disable certificate verification.
// Different Cloudflare certificates may require updating this root certificate.
// Public Let's Encrypt ISRG Root X1 trust anchor. Uses TLS certificate validation.
// If your domain uses another issuer, supply the matching trusted CA instead.
static const char CLOUD_ROOT_CA[] PROGMEM = R"PS_ROOT_CA(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)PS_ROOT_CA";
#endif

#ifndef PARKSENSE_AP_PASSWORD
#define PARKSENSE_AP_PASSWORD "123456789"
#endif


#if CLOUD_UPLINK_ENABLED
// Prevent accidental connection using example credentials or a too-short token.
// This validation never logs or transmits passwords or tokens.
bool cloudCredentialsValid() {
  const size_t ssidLen = strlen(CLOUD_WIFI_SSID);
  const size_t passLen = strlen(CLOUD_WIFI_PASSWORD);
  const size_t tokenLen = strlen(CLOUD_DEVICE_TOKEN);
  return ssidLen >= 1 && ssidLen <= 32 &&
         passLen >= 8 && passLen <= 63 &&
         tokenLen >= 24 &&
         strncmp(CLOUD_WIFI_SSID, "ISI_", 4) != 0 &&
         strncmp(CLOUD_WIFI_PASSWORD, "ISI_", 4) != 0 &&
         strncmp(CLOUD_DEVICE_TOKEN, "ISI_", 4) != 0 &&
         strcmp(CLOUD_WIFI_PASSWORD, CLOUD_DEVICE_TOKEN) != 0;
}
#endif

// IMPORTANT: Matches the latest user-supplied main.cpp wiring.
// Active-HIGH LEDs and buzzer via a suitable transistor/driver.
constexpr int RED_LED_PIN = 18;
constexpr int YELLOW_LED_PIN = 19;
constexpr int GREEN_LED_PIN = 21;
constexpr int BUZZER_PIN = 13;
constexpr int POWER_SWITCH_PIN = 14;  // latching switch GPIO14-to-GND = ON
constexpr int ENC_CLK = 25;
constexpr int ENC_DT = 26;
constexpr int ENC_SW = 27;
constexpr bool LED_ACTIVE_LOW = false;
constexpr bool BUZZER_ACTIVE_LOW = false;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
TinyGPSPlus gps;
int speeds[4] = {0,0,0,0};
int distances[4] = {400,400,400,400};
int humans[4] = {0,0,0,0};
unsigned long lastPodHeardTime[4] = {0,0,0,0};
portMUX_TYPE podMux = portMUX_INITIALIZER_UNLOCKED;
volatile int liveGpsSpeed = 0;
unsigned long lastGpsUpdateTime = 0;
volatile bool systemEnabled = false;
volatile int speedLimit = 15;
volatile int currentWifiChannel = 0;  // actual Wi-Fi channel, never guessed
int lastClkState = LOW, lastSwState = HIGH;
unsigned long lastEncoderPoll = 0, lastButtonPress = 0;

// Must match firmware/esp32-sensor packet ABI exactly.
typedef struct struct_message {
  int podID;
  int speedKmh;
  int distanceCm;
  int humanDetected;
} struct_message;

void checkEncoder() {
  if (millis() - lastEncoderPoll < 2) return;
  lastEncoderPoll = millis();
  int currentClkState = digitalRead(ENC_CLK);
  if (lastClkState == HIGH && currentClkState == LOW) {
    if (digitalRead(ENC_DT) != currentClkState) {
      if (speedLimit < 100) speedLimit++;
    } else {
      if (speedLimit > 5) speedLimit--;
    }
  }
  lastClkState = currentClkState;
  int currentSwState = digitalRead(ENC_SW);
  if (currentSwState == LOW && lastSwState == HIGH && millis() - lastButtonPress > 500) {
    speedLimit = 15;
    lastButtonPress = millis();
  }
  lastSwState = currentSwState;
}

bool alertState = false;
bool previousHumanDetected = false;
unsigned long motionBeepStart = 0;
bool motionBeepSequenceActive = false;
bool motionAlertPending = false;
constexpr unsigned long MOTION_BEEP_DURATION_MS = 200;
constexpr unsigned long MOTION_BEEP_GAP_MS = 1000;

void setLed(int pin, bool on) { digitalWrite(pin, (on != LED_ACTIVE_LOW) ? HIGH : LOW); }
void setBuzzer(bool on) { digitalWrite(BUZZER_PIN, (on != BUZZER_ACTIVE_LOW) ? HIGH : LOW); }
void allAlertsOff() {
  setLed(RED_LED_PIN, false);
  setLed(YELLOW_LED_PIN, false);
  setLed(GREEN_LED_PIN, false);
  setBuzzer(false);
}
bool pulseOn(unsigned long now, unsigned long periodMs, unsigned long onMs) {
  return periodMs > 0 && (now % periodMs) < onMs;
}
void checkPowerSwitch() { systemEnabled = digitalRead(POWER_SWITCH_PIN) == LOW; }

// Firmware output priorities copied from the user's latest main.cpp.
void updateHardwareAlerts() {
  const unsigned long now = millis();
  if (!systemEnabled) {
    allAlertsOff();
    alertState = false;
    previousHumanDetected = false;
    motionBeepSequenceActive = false;
    motionAlertPending = false;
    return;
  }
  int minDistance = 400;
  bool anyHumanDetected = false;
  bool anyPodOnline = false;
  portENTER_CRITICAL(&podMux);
  for (int i = 0; i < 4; ++i) {
    const bool online = lastPodHeardTime[i] != 0 && now - lastPodHeardTime[i] <= 2500;
    if (!online) continue;
    anyPodOnline = true;
    if (distances[i] >= 2 && distances[i] <= 400 && distances[i] < minDistance) minDistance = distances[i];
    if (humans[i] == 1) anyHumanDetected = true;
  }
  portEXIT_CRITICAL(&podMux);
  if (anyHumanDetected && !previousHumanDetected) motionAlertPending = true;
  previousHumanDetected = anyHumanDetected;

  const int measuredSpeed = liveGpsSpeed;
  const int speedDiff = measuredSpeed - speedLimit;
  const bool overSpeed = measuredSpeed > speedLimit && measuredSpeed > 0;
  const bool criticalDistance = minDistance < 15;
  const bool nearDistance = !criticalDistance && minDistance < 35;
  const bool cautionDistance = !criticalDistance && !nearDistance && minDistance < 55;
  const bool higherPriorityAlert = criticalDistance || overSpeed || nearDistance || cautionDistance;
  if (higherPriorityAlert && motionBeepSequenceActive) motionBeepSequenceActive = false;

  bool redOn=false, yellowOn=false, greenOn=false, buzzerOn=false;
  if (criticalDistance) {
    redOn = true; buzzerOn = true;
  } else if (overSpeed) {
    const unsigned long period = (unsigned long)max(200, 600 - min(speedDiff, 40) * 10);
    redOn = pulseOn(now, period, period/2);
    buzzerOn = redOn;
  } else if (nearDistance) {
    redOn = pulseOn(now, 300, 150);
    buzzerOn = redOn;
  } else if (cautionDistance) {
    yellowOn = true;
    buzzerOn = pulseOn(now, 1000, 100);
  } else {
    if (motionAlertPending && !motionBeepSequenceActive) {
      motionBeepStart = now;
      motionBeepSequenceActive = true;
    }
    if (motionBeepSequenceActive) {
      const unsigned long elapsed = now - motionBeepStart;
      if (elapsed < MOTION_BEEP_DURATION_MS ||
          (elapsed >= MOTION_BEEP_DURATION_MS + MOTION_BEEP_GAP_MS &&
           elapsed < 2 * MOTION_BEEP_DURATION_MS + MOTION_BEEP_GAP_MS)) buzzerOn = true;
      if (elapsed >= 2*MOTION_BEEP_DURATION_MS+MOTION_BEEP_GAP_MS) {
        motionBeepSequenceActive = false;
        motionAlertPending = false;
      }
    }
    if (anyHumanDetected) yellowOn = true;
    if (!motionBeepSequenceActive && !motionAlertPending && anyPodOnline) greenOn = true;
  }
  setLed(RED_LED_PIN, redOn);
  setLed(YELLOW_LED_PIN, yellowOn);
  setLed(GREEN_LED_PIN, greenOn);
  setBuzzer(buzzerOn);
}

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *packet, int len) {
#else
void OnDataRecv(const uint8_t *mac, const uint8_t *packet, int len) {
#endif
  if (len != sizeof(struct_message)) return;
  struct_message data;
  memcpy(&data, packet, sizeof(data));
  if (data.podID < 0 || data.podID >= 4 ||
      data.distanceCm < 2 || data.distanceCm > 400 ||
      data.speedKmh < 0 || data.speedKmh > 300 ||
      (data.humanDetected != 0 && data.humanDetected != 1)) return;
  portENTER_CRITICAL(&podMux);
  speeds[data.podID] = data.speedKmh;
  distances[data.podID] = data.distanceCm;
  humans[data.podID] = data.humanDetected;
  lastPodHeardTime[data.podID] = millis();
  portEXIT_CRITICAL(&podMux);
}

// Shared JSON schema for local WS and Coolify uplink; GPS speed never falls back
// to "speedKmh" from any pod. Offline pods are -1 after 2 seconds.
void makeTelemetry(JsonDocument &doc) {
  const unsigned long now = millis();
  int dist[4], motion[4];
  portENTER_CRITICAL(&podMux);
  for (int i=0; i<4; ++i) {
    const bool online = lastPodHeardTime[i] != 0 && now-lastPodHeardTime[i] <= 2000;
    dist[i] = online ? distances[i] : -1;
    motion[i] = online ? humans[i] : 0;
  }
  portEXIT_CRITICAL(&podMux);
  doc["maxSpeed"] = (int)liveGpsSpeed;
  doc["speedLimit"] = (int)speedLimit;
  doc["systemEnabled"] = (bool)systemEnabled;
  doc["remoteControl"] = false; // local WebSocket remains read-only
  doc["wifiChannel"] = (int)currentWifiChannel;
  const char *dkeys[4] = {"distF","distR","distL","distRefR"};
  const char *hkeys[4] = {"humanF","humanR","humanL","humanRefR"};
  for (int i=0;i<4;++i) { doc[dkeys[i]]=dist[i]; doc[hkeys[i]]=motion[i]; }
}
void broadcastTelemetry() {
  static unsigned long lastAt = 0;
  if (millis()-lastAt < 150 || ws.count()==0) return;
  lastAt = millis();
  JsonDocument doc;
  makeTelemetry(doc);
  String json;
  serializeJson(doc,json);
  ws.textAll(json);
}

const char index_html[] PROGMEM = R"PARKSENSE_HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="theme-color" content="#0b1119">
  <title>ParkSense — Parking Speed Limiter Monitor</title>
  <style>
    :root{--bg:#0b1119;--panel:#131c27;--panel2:#17222e;--border:#283442;--text:#e9f1f5;--muted:#8b9ba8;--teal:#57decb;--green:#7ee1ae;--amber:#f5bf68;--red:#fa7d7c;--grey:#798695;--r:18px}
    *{box-sizing:border-box}
    body{background:var(--bg);color:var(--text);margin:0;font-family:Inter,ui-sans-serif,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;font-size:14px;min-height:100vh}
    button,input{font:inherit}
    button{cursor:pointer}button:disabled{opacity:.45;cursor:not-allowed}
    .app{max-width:1430px;margin:0 auto;padding:24px clamp(14px,3vw,40px) 38px}
    .top{display:flex;justify-content:space-between;align-items:center;gap:14px;border-bottom:1px solid var(--border);padding:0 0 22px;flex-wrap:wrap}
    .brand{display:flex;align-items:center;gap:13px}
    .brandmark{width:39px;height:39px;border-radius:11px;border:1px solid #3c5e63;background:#153234;display:grid;place-items:center;color:var(--teal)}
    .brand-title{font-size:17px;font-weight:780;letter-spacing:-.6px;line-height:1.1}.brand-caption{color:var(--muted);font-size:11px;letter-spacing:1.9px;margin-top:5px;font-weight:650}
    .top-right{display:flex;align-items:center;gap:8px;flex-wrap:wrap}.chip{display:inline-flex;align-items:center;gap:7px;border:1px solid var(--border);padding:8px 11px;border-radius:999px;color:#bacbd3;background:#121a24;font-size:11px;letter-spacing:.8px;font-weight:800}
    .dot{display:inline-block;flex:none;width:7px;height:7px;background:#7e8c98;border-radius:50%}.dot.good{background:var(--green);box-shadow:0 0 0 4px #7ee1ae19}.dot.demo{background:var(--amber)}.dot.bad{background:var(--red)}
    .outline-button,.solid-button{border:1px solid var(--border);color:#dce7ed;border-radius:10px;background:#192431;padding:10px 15px;font-weight:700;transition:.15s}.outline-button:hover{border-color:#57727b;background:#1e2e3a}.solid-button{background:var(--teal);color:#08202a;border-color:var(--teal)}.solid-button:hover{background:#87f0e0}.solid-button:disabled{background:#61747a;color:#182029}
    .intro{display:flex;align-items:end;justify-content:space-between;gap:12px;margin:27px 0 20px;flex-wrap:wrap}.eyebrow,.micro{color:var(--muted);font-size:10px;letter-spacing:2px;font-weight:800;text-transform:uppercase}.intro h1{font-size:clamp(23px,2.3vw,34px);font-weight:760;line-height:1.12;letter-spacing:-1.1px;margin:7px 0 6px}.intro p{color:var(--muted);margin:0;font-size:12px}.timestamp{color:#8b9ba8;font-size:12px;font-variant-numeric:tabular-nums}
    .cards{display:grid;grid-template-columns:minmax(225px,1fr) minmax(330px,1.25fr) minmax(270px,1fr);gap:16px;align-items:stretch}
    .panel{background:var(--panel);border:1px solid var(--border);border-radius:var(--r);overflow:hidden;min-width:0}
    .panel-inner{padding:21px 22px}.panel-header{display:flex;align-items:center;justify-content:space-between;gap:10px}.panel-title{font-weight:740;font-size:13px;color:#e0ebef}.subnote{color:var(--muted);font-size:11px;line-height:1.5}
    .speedpanel{min-height:440px;display:flex;flex-direction:column}.speed-panel-top{display:flex;align-items:center;justify-content:space-between}.live-pin{font-size:10px;background:#183831;border:1px solid #2a5446;color:var(--green);border-radius:6px;padding:5px 8px;font-weight:800;letter-spacing:.8px}
    .speed-number{font-size:clamp(76px,9vw,124px);letter-spacing:-10px;font-weight:650;line-height:1.1;font-variant-numeric:tabular-nums;margin-top:30px}.speed-number.warm{color:var(--amber)}.speed-number.hot{color:var(--red)}.speed-unit{font-size:13px;color:var(--muted);font-weight:750;letter-spacing:1.8px;margin:4px 0 27px}
    .speed-meter{height:7px;border-radius:100px;background:#2d3945;position:relative;overflow:hidden}.speed-fill{width:0%;height:100%;border-radius:100px;background:var(--teal);transition:width .35s,background .25s}
    .meter-ticks{display:flex;justify-content:space-between;font-size:10px;color:var(--muted);margin-top:10px;font-variant-numeric:tabular-nums}
    .speedfooter{margin-top:auto;border-top:1px solid var(--border);padding-top:18px;display:flex;justify-content:space-between;align-items:center;gap:10px}.limit-num{font-size:37px;letter-spacing:-2px;font-weight:750;margin-top:4px;line-height:1.1}.limit-num small{font-size:12px;letter-spacing:0;color:var(--muted);font-weight:600}.control-row{display:flex;gap:5px;align-items:center}.square-button{background:#1f2d3a;border:1px solid #364858;color:#d8e9ee;width:34px;height:34px;border-radius:8px;font-weight:800;font-size:18px}.square-button:hover{border-color:var(--teal)}.control-hint{font-size:10px;margin-top:9px;color:var(--muted);max-width:245px;line-height:1.45}
    .carpanel{min-height:440px;background:radial-gradient(ellipse at 50% 50%,#1b2b36 0%,#131c27 57%);position:relative}.car-head{padding:21px 22px 4px}.car-arena{position:relative;height:341px;display:grid;place-items:center;overflow:hidden}.radar-lines{position:absolute;width:290px;height:290px;border:1px dashed #2b4150;border-radius:50%;box-shadow:0 0 0 47px #0000,0 0 0 48px #21313b66,0 0 0 89px #0000,0 0 0 90px #1d2c3755;opacity:.9;pointer-events:none}.radar-lines:before,.radar-lines:after{content:"";position:absolute;background:#233743}.radar-lines:before{height:1px;width:100%;left:0;top:50%}.radar-lines:after{width:1px;height:100%;left:50%;top:0}
    .vehicle{width:116px;height:240px;filter:drop-shadow(0 12px 22px #0008);position:absolute;left:50%;top:50%;transform:translate(-50%,-50%)}.vehicle svg{width:100%;height:100%;display:block}.sensor-pill{position:absolute;min-width:99px;background:#101923e8;border:1px solid #3e5361;color:#dcecf1;border-radius:10px;padding:8px 11px;text-align:center;z-index:2;box-shadow:0 4px 14px #0006;transition:border-color .25s,background .25s}.sensor-pill .dir{font-size:9px;font-weight:800;letter-spacing:1.3px;color:#94a4af}.sensor-pill .reading{font-size:17px;font-weight:800;font-variant-numeric:tabular-nums;margin-top:2px}.sensor-pill .reading small{font-size:10px;color:var(--muted)}.sensor-pill.alert-red{border-color:var(--red);background:#392027}.sensor-pill.alert-yellow{border-color:var(--amber);background:#37301f}.sensor-pill.alert-offline{border-color:#37414b;opacity:.65}.sensor-pill[data-dir="F"]{top:4px;left:50%;transform:translateX(-50%)}.sensor-pill[data-dir="R"]{bottom:2px;left:50%;transform:translateX(-50%)}.sensor-pill[data-dir="L"]{left:max(7px,calc(50% - 199px));top:50%;transform:translateY(-50%)}.sensor-pill[data-dir="Rt"]{right:max(7px,calc(50% - 199px));top:50%;transform:translateY(-50%)}
    .vehicle-caption{padding:0 22px 17px;color:var(--muted);font-size:10px;display:flex;gap:13px;justify-content:center;flex-wrap:wrap}.legend{display:flex;gap:6px;align-items:center}.legend i{display:inline-block;width:7px;height:7px;border-radius:2px;background:var(--green)}.legend i.warning{background:var(--amber)}.legend i.danger{background:var(--red)}
    .zonepanel{min-height:440px;display:flex;flex-direction:column}.zones-list{padding:10px 18px 4px}.zone-row{display:flex;align-items:center;gap:11px;border-bottom:1px solid #293543;padding:13px 0}.zone-row:last-child{border-bottom:none}.zone-icon{width:36px;height:36px;border:1px solid #384957;border-radius:9px;display:grid;place-items:center;flex-shrink:0;color:var(--teal);font-size:12px;font-weight:850}.zone-label{font-size:12px;font-weight:750}.zone-status{font-size:10px;color:var(--muted);margin-top:5px;white-space:nowrap}.zone-right{margin-left:auto;text-align:right}.zone-dist{font-size:17px;font-weight:780;font-variant-numeric:tabular-nums}.zone-dist small{font-size:10px;color:var(--muted);font-weight:550}.state-tag{color:var(--green);font-size:9px;letter-spacing:.5px;font-weight:800;margin-top:5px}.state-tag.warn{color:var(--amber)}.state-tag.danger{color:var(--red)}.state-tag.offline{color:var(--grey)}.zones-note{margin:auto 18px 18px;padding:12px;border-radius:9px;background:#1c2933;color:#9fb0ba;font-size:11px;line-height:1.55}
    .below{display:grid;grid-template-columns:1fr 1fr;gap:16px;margin-top:16px}.alert-box{border-left:3px solid var(--green)}.alert-box.warn{border-color:var(--amber)}.alert-box.danger{border-color:var(--red)}.alert-content{display:flex;gap:15px;align-items:center;margin-top:16px}.alert-symbol{height:49px;width:49px;border-radius:12px;display:grid;place-items:center;background:#203c37;color:var(--green);font-size:23px;font-weight:900}.alert-box.warn .alert-symbol{background:#403425;color:var(--amber)}.alert-box.danger .alert-symbol{background:#472930;color:var(--red)}.alert-headline{font-size:21px;letter-spacing:-.7px;font-weight:740}.alert-description{color:var(--muted);margin-top:6px;font-size:11px;line-height:1.5}
    .events-list{margin-top:12px;display:flex;flex-direction:column;gap:11px;max-height:155px;overflow:auto}.event{display:flex;justify-content:space-between;gap:10px;font-size:11px;color:#bdcbd4;line-height:1.4;border-bottom:1px solid #273543;padding-bottom:9px}.event:last-child{border:0}.event time{font-variant-numeric:tabular-nums;white-space:nowrap;color:#748c9b;font-size:10px}
    .connectpanel{margin-top:16px}.connect-layout{display:flex;align-items:end;gap:12px;flex-wrap:wrap;margin-top:14px}.field{flex:1;min-width:250px}.field label{display:block;color:var(--muted);font-size:10px;font-weight:800;letter-spacing:1.4px;margin-bottom:8px}.url-input{background:#101922;border:1px solid #354756;outline:0;color:#deedf2;border-radius:10px;padding:12px 14px;width:100%;font-size:13px;font-variant-numeric:tabular-nums}.url-input:focus{border-color:var(--teal)}.connect-helper{color:var(--muted);font-size:11px;margin-top:11px;line-height:1.55}.connect-actions{display:flex;gap:8px;align-items:center;flex-wrap:wrap}.tiny-foot{font-size:10px;color:#79909e;text-align:center;margin-top:19px;line-height:1.65}
    @media(min-width:1100px){.below{grid-template-columns:1fr 1fr}.car-arena{height:341px}}
    @media(max-width:1080px){.cards{grid-template-columns:1fr 1.2fr}.zonepanel{grid-column:1 / -1;min-height:unset}.zones-list{display:grid;grid-template-columns:1fr 1fr;gap:0 24px}.zone-row:nth-child(3){border-bottom:none}.zones-note{margin-top:12px}.carpanel,.speedpanel{min-height:420px}}
    @media(max-width:650px){.app{padding:17px 13px 26px}.top{padding-bottom:15px}.brand-title{font-size:15px}.brand-caption{font-size:9px}.chip{font-size:10px;padding:7px 10px}.top-right{width:100%;justify-content:space-between}.intro{margin:23px 0 16px}.intro h1{font-size:25px}.cards,.below{grid-template-columns:1fr}.speedpanel{min-height:345px}.speed-number{margin-top:16px;font-size:92px}.speed-unit{margin-bottom:23px}.carpanel{min-height:428px}.car-arena{height:340px}.zonepanel{grid-column:auto}.zones-list{display:block}.zone-row:nth-child(3){border-bottom:1px solid #293543}.connect-layout,.connect-actions{align-items:stretch}.connect-actions{width:100%}.connect-actions button{flex:1}.below{gap:14px}.sensor-pill[data-dir="L"]{left:5px}.sensor-pill[data-dir="Rt"]{right:5px}.sensor-pill{min-width:87px;padding:7px 8px}.sensor-pill .reading{font-size:15px}.vehicle{height:219px;width:105px}.radar-lines{height:258px;width:258px}}
    @media(max-width:370px){.sensor-pill{min-width:77px}.sensor-pill .reading{font-size:13px}.car-arena{height:325px}.vehicle{height:208px}.speed-number{font-size:80px}.panel-inner{padding:18px}}
    @media(prefers-reduced-motion:reduce){*{transition:none!important;animation:none!important}}
  </style>
</head>
<body>
<div class="app">
  <header class="top">
    <div class="brand">
      <div class="brandmark" aria-hidden="true"><svg width="25" height="25" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round"><path d="M4 12a8 8 0 0 1 16 0M6 19l2-3 2 2 3-5 2 2 3-4"/><circle cx="18" cy="5" r="1" fill="currentColor" stroke="none"/></svg></div>
      <div><div class="brand-title">ParkSense <span style="color:var(--teal)">/</span> Monitor</div><div class="brand-caption">PARKING SPEED LIMITER SYSTEM</div></div>
    </div>
    <div class="top-right">
      <div class="chip"><span class="dot demo" id="modeDot"></span><span id="modeText">DEMO MODE</span></div>
      <div class="chip"><span class="dot bad" id="connectionDot"></span><span id="connectionText">DEVICE NOT CONNECTED</span></div>
      <button class="outline-button" type="button" id="toConnection">Connection settings ↗</button>
    </div>
  </header>
  <section class="intro">
    <div><div class="eyebrow">SYSTEM OVERVIEW / 01</div><h1>Live vehicle monitoring</h1><p>Four-point obstacle detection, movement alerts and speed threshold monitoring.</p></div>
    <div class="timestamp" id="lastUpdate">Last update — demonstration</div>
  </section>
  <main class="cards">
    <section class="panel speedpanel" aria-label="Vehicle speed">
      <div class="panel-inner speedpanel" style="border:0;border-radius:0">
        <div class="speed-panel-top"><div class="panel-title">Vehicle speed</div><span class="live-pin" id="readingType">SIMULATED</span></div>
        <div class="speed-number" id="speedValue">08</div>
        <div class="speed-unit">KM / H</div>
        <div class="speed-meter" role="meter" aria-label="Speed relative to maximum" aria-valuemin="0" aria-valuemax="100" aria-valuenow="8" id="speedMeter"><div class="speed-fill" id="speedFill" style="width:8%"></div></div>
        <div class="meter-ticks"><span>0</span><span id="midTick">50</span><span>100 km/h</span></div>
        <div class="speedfooter"><div><div class="micro">Speed alert threshold</div><div class="limit-num"><span id="limitValue">15</span> <small>km/h</small></div></div><div class="control-row"><button id="limitMinus" class="square-button" title="Decrease threshold" aria-label="Decrease threshold">−</button><button id="limitPlus" class="square-button" title="Increase threshold" aria-label="Increase threshold">+</button><button id="limitReset" class="square-button" title="Reset threshold to 15" aria-label="Reset threshold" style="font-size:15px">↺</button></div></div>
        <div class="control-hint" id="controlHint">Demo controls active. In live mode, the original firmware uses the rotary encoder.</div>
      </div>
    </section>
    <section class="panel carpanel" aria-label="Surround view sensor overview">
      <div class="car-head"><div class="panel-header"><div class="panel-title">360° surround view</div><span class="micro" id="activePods">4 / 4 PODS</span></div><p class="subnote">Real-time distance readings from all four directions</p></div>
      <div class="car-arena">
        <div class="radar-lines"></div>
        <div class="vehicle" aria-hidden="true"><svg viewBox="0 0 120 250" xmlns="http://www.w3.org/2000/svg">
          <rect x="1" y="41" width="16" height="42" rx="6" fill="#080f16" stroke="#506274" stroke-width="2"/>
          <rect x="103" y="41" width="16" height="42" rx="6" fill="#080f16" stroke="#506274" stroke-width="2"/>
          <rect x="1" y="167" width="16" height="41" rx="6" fill="#080f16" stroke="#506274" stroke-width="2"/>
          <rect x="103" y="167" width="16" height="41" rx="6" fill="#080f16" stroke="#506274" stroke-width="2"/>
          <path d="M42 8Q60 0 78 8L99 33Q104 42 104 56L103 208Q103 225 90 237Q60 250 30 237Q17 225 17 208L16 56Q16 42 21 33Z" fill="#516677" stroke="#91a4b1" stroke-width="2"/>
          <path d="M33 20Q60 9 87 20L94 59L26 59Z" fill="#13232e" stroke="#92a8b5" stroke-width="1.5"/>
          <path d="M26 69L94 69L94 173L26 173Z" fill="#2a414f" stroke="#8197a5" stroke-width="1.3"/>
          <path d="M29 187L91 187L87 226Q60 235 33 226Z" fill="#13232e" stroke="#92a8b5" stroke-width="1.5"/>
          <path d="M16 57L7 70L16 76M104 57L113 70L104 76" fill="#6e8997" stroke="#95aab4" stroke-width="1"/>
          <path d="M22 36H33M87 36H98" stroke="#c8e8df" stroke-width="5" stroke-linecap="round"/>
          <path d="M23 219H37M83 219H97" stroke="#fa7773" stroke-width="5" stroke-linecap="round"/>
          <path d="M42 119h36" stroke="#597786" stroke-width="1.5" opacity=".65"/>
        </svg></div>
        <div class="sensor-pill" data-dir="F" id="pillF"><div class="dir">FRONT</div><div class="reading" id="pillFValue">85 <small>cm</small></div></div>
        <div class="sensor-pill" data-dir="R" id="pillR"><div class="dir">REAR</div><div class="reading" id="pillRValue">120 <small>cm</small></div></div>
        <div class="sensor-pill" data-dir="L" id="pillL"><div class="dir">LEFT</div><div class="reading" id="pillLValue">64 <small>cm</small></div></div>
        <div class="sensor-pill" data-dir="Rt" id="pillRt"><div class="dir">RIGHT</div><div class="reading" id="pillRtValue">95 <small>cm</small></div></div>
      </div>
      <div class="vehicle-caption"><span class="legend"><i></i> Normal</span><span class="legend"><i class="warning"></i> Caution &lt; 55 cm</span><span class="legend"><i class="danger"></i> Danger &lt; 35 cm</span></div>
    </section>
    <section class="panel zonepanel" aria-label="Individual sensor status">
      <div style="padding:21px 19px 3px" class="panel-header"><div class="panel-title">Sensor status</div><span class="micro">ESP-NOW PODS</span></div>
      <div class="zones-list">
        <div class="zone-row" id="zoneF"><div class="zone-icon">F</div><div><div class="zone-label">Front sensor</div><div class="zone-status" id="motionF">No movement</div></div><div class="zone-right"><div class="zone-dist" id="distF">85 <small>cm</small></div><div class="state-tag" id="statusF">CLEAR</div></div></div>
        <div class="zone-row" id="zoneR"><div class="zone-icon">R</div><div><div class="zone-label">Rear sensor</div><div class="zone-status" id="motionR">No movement</div></div><div class="zone-right"><div class="zone-dist" id="distR">120 <small>cm</small></div><div class="state-tag" id="statusR">CLEAR</div></div></div>
        <div class="zone-row" id="zoneL"><div class="zone-icon">L</div><div><div class="zone-label">Left sensor</div><div class="zone-status" id="motionL">No movement</div></div><div class="zone-right"><div class="zone-dist" id="distL">64 <small>cm</small></div><div class="state-tag" id="statusL">CLEAR</div></div></div>
        <div class="zone-row" id="zoneRt"><div class="zone-icon">R</div><div><div class="zone-label">Right sensor</div><div class="zone-status" id="motionRt">No movement</div></div><div class="zone-right"><div class="zone-dist" id="distRt">95 <small>cm</small></div><div class="state-tag" id="statusRt">CLEAR</div></div></div>
      </div>
      <div class="zones-note" id="parkingModeIndicator">Parking mode: OFF</div><div class="zones-note">Sensor distance under 15 cm triggers the highest alert level. A pod with no data for over 2 seconds is shown as offline.</div>
    </section>
  </main>
  <section class="below">
    <article class="panel alert-box" id="alertBox"><div class="panel-inner"><div class="panel-header"><div class="panel-title">System alert</div><span class="micro">LIVE ASSESSMENT</span></div><div class="alert-content"><div class="alert-symbol" id="alertSymbol">✓</div><div><div class="alert-headline" id="alertHeadline">Normal operation</div><div class="alert-description" id="alertDescription">Vehicle speed and detected distances are within the monitored thresholds.</div></div></div></div></article>
    <article class="panel"><div class="panel-inner"><div class="panel-header"><div class="panel-title">Event log</div><span class="micro">RECENT ACTIVITY</span></div><div class="events-list" id="eventList" aria-live="polite"><div class="event"><span>Preview started in demo mode</span><time>—</time></div></div></div></article>
  </section>
  <section class="panel connectpanel" id="connectionSettings"><div class="panel-inner"><div class="panel-header"><div class="panel-title">Device connection</div><span class="micro">LOCAL WEBSOCKET</span></div><div class="connect-layout"><div class="field"><label for="wsUrl">ESP32 WEBSOCKET ADDRESS</label><input class="url-input" type="text" id="wsUrl" value="ws://192.168.4.1/ws" spellcheck="false" autocomplete="off" placeholder="ws://192.168.4.1/ws"></div><div class="connect-actions"><button class="solid-button" type="button" id="connectBtn">Connect to ESP32</button><button class="outline-button" type="button" id="demoBtn">Use demo</button></div></div><div class="connect-helper" id="connectHelp">To use hardware, connect your phone or laptop to the ESP32 Wi-Fi network <strong>Monitor Dashboard</strong>, then connect. When this page is served by the ESP32 itself, it connects automatically.</div></div></section>
  <footer class="tiny-foot">FYP PROTOTYPE · ESP32 + ESP-NOW + GPS + ULTRASONIC + PIR · Monitoring and warnings only — no physical throttle or brake control is implemented.</footer>
</div>
<script>
(function(){
  'use strict';
  const $ = id => document.getElementById(id);
  const zones=[['F','distF','humanF'],['R','distR','humanR'],['L','distL','humanL'],['Rt','distRefR','humanRefR']];
  const DEFAULT_HOST='192.168.4.1';
  let mode='demo', socket=null, wantsConnection=false, reconnectTimer=null, lastTelemetry=0, remoteControl=false, lastAlert='', demoStep=0, lastSensorState={};
  let telemetry={systemEnabled:false,maxSpeed:8,speedLimit:15,distF:85,distR:120,distL:64,distRefR:95,humanF:0,humanR:0,humanL:0,humanRefR:0};
  let eventLog=[];
  if(location.protocol==='http:' && location.hostname && location.hostname!=='localhost' && location.hostname!=='127.0.0.1'){
    $('wsUrl').value='ws://'+location.host+'/ws';
  } else if(location.protocol==='https:' && location.hostname && location.hostname!=='localhost') {
    $('wsUrl').value='wss://'+location.host+'/ws';
  }
  function stamp(){return new Date().toLocaleTimeString([], {hour:'2-digit',minute:'2-digit',second:'2-digit',hour12:false});}
  function log(message){if(eventLog[0] && eventLog[0].message===message)return;eventLog.unshift({message,time:stamp()});eventLog=eventLog.slice(0,5);$('eventList').replaceChildren(...eventLog.map(x=>{let e=document.createElement('div'),s=document.createElement('span'),t=document.createElement('time');e.className='event';s.textContent=x.message;t.textContent=x.time;e.append(s,t);return e}));}
  function setMode(next,status){mode=next;$('modeText').textContent=next==='demo'?'DEMO MODE':'LIVE MODE';$('modeDot').className='dot '+(next==='demo'?'demo':'good');$('readingType').textContent=next==='demo'?'SIMULATED':'ESP32 FEED';$('connectionDot').className='dot '+(status==='connected'?'good':'bad');$('connectionText').textContent=next==='demo'?'DEVICE NOT CONNECTED':status==='connected'?'DEVICE CONNECTED':status==='connecting'?'CONNECTING…':'DEVICE DISCONNECTED';$('lastUpdate').textContent=next==='demo'?'Last update — demonstration':'Last update — awaiting data';updateControls();}
  function updateControls(){let enabled=mode==='demo'||(mode==='live'&&remoteControl&&socket&&socket.readyState===WebSocket.OPEN);['limitMinus','limitPlus','limitReset'].forEach(id=>$(id).disabled=!enabled);$('controlHint').textContent=mode==='demo'?'Demo controls only — simulated threshold.':remoteControl?'Controls update the ESP32 speed alert threshold (5–100 km/h).':'Adjust speed limit using the ESP32 rotary encoder. Remote controls require the supplied updated firmware.';}
  function severity(dist,human){if(dist===-1||!Number.isFinite(dist)||dist<2)return'offline';if(dist<35)return'danger';if(dist<55||human===1)return'warn';return'good';}
  function formatDistance(dist){if(dist===-1||!Number.isFinite(dist)||dist<2)return 'OFFLINE';if(dist>=395)return '≥395 <small>cm</small>';return String(Math.round(dist))+' <small>cm</small>';}
  function render(){
    let speed=Number(telemetry.maxSpeed)||0,limit=Number(telemetry.speedLimit)||15;
    speed=Math.max(0,Math.min(999,speed));limit=Math.max(5,Math.min(100,limit));
    $('parkingModeIndicator').textContent='Parking mode: '+(telemetry.systemEnabled===true?'ON — hardware alerts enabled':'OFF — hardware alerts disabled');$('speedValue').textContent=String(Math.round(speed)).padStart(2,'0');$('limitValue').textContent=limit;
    $('speedFill').style.width=Math.min(100,(speed/100)*100)+'%';$('speedMeter').setAttribute('aria-valuenow',Math.min(100,speed));
    $('speedValue').className='speed-number'+(speed>limit?' hot':speed>limit*.8?' warm':'');
    $('speedFill').style.background=speed>limit?'var(--red)':speed>limit*.8?'var(--amber)':'var(--teal)';
    let active=0,min=400,humanFound=false,anyDanger=false,anyWarn=false;
    zones.forEach(([key,distKey,humanKey])=>{
      const dist=Number(telemetry[distKey]),human=Number(telemetry[humanKey])===1?1:0, sev=severity(dist,human);
      if(sev!=='offline')active++;
      if(dist>=2&&dist<395&&sev!=='offline')min=Math.min(min,dist);
      if(human)humanFound=true;if(sev==='danger')anyDanger=true;if(sev==='warn')anyWarn=true;
      const label=sev==='offline'?'OFFLINE':human?'MOTION DETECTED':dist>=395?'OUT OF RANGE':dist<15?'CRITICAL':dist<35?'DANGER':dist<55?'CAUTION':'CLEAR';
      const pill=$('pill'+key);pill.className='sensor-pill'+(sev==='danger'?' alert-red':sev==='warn'?' alert-yellow':sev==='offline'?' alert-offline':'');
      $('pill'+key+'Value').innerHTML=formatDistance(dist);$('dist'+key).innerHTML=formatDistance(dist);
      $('motion'+key).textContent=sev==='offline'?'No pod telemetry':human?'PIR: motion detected':'PIR: no movement';
      const tag=$('status'+key);tag.textContent=label;tag.className='state-tag'+(sev==='offline'?' offline':sev==='danger'?' danger':sev==='warn'?' warn':'');
      if(lastSensorState[key]&&lastSensorState[key]!==label&&(human||sev==='danger'||sev==='offline')&&mode==='live')log((key==='Rt'?'RIGHT':key==='F'?'FRONT':key==='R'?'REAR':'LEFT')+' sensor: '+label.toLowerCase());
      lastSensorState[key]=label;
    });
    $('activePods').textContent=active+' / 4 PODS';
    let headline='Normal operation',message='Vehicle speed and detected distances are within the monitored thresholds.',tone='',symbol='✓';
    if(mode==='live'&&lastTelemetry&&telemetry.systemEnabled!==true){headline='Parking mode OFF';message='The GPIO14 latching switch disables LED and buzzer alerts. Sensor data can still be monitored.';tone='warn';symbol='!';}
    else if(active===0){headline='No sensor signal';message='No ESP-NOW pod data is available. Check sensor power and communications.';tone='warn';symbol='!';}
    else if(min<15){headline='Immediate obstacle alert';message='An obstacle is closer than 15 cm. The ESP32 activates its continuous red LED and buzzer when parking mode is enabled.';tone='danger';symbol='!';}
    else if(speed>limit){headline='Speed limit exceeded';message='Vehicle speed is above the '+limit+' km/h warning threshold.';tone='danger';symbol='!';}
    else if(min<35){headline='Obstacle very close';message='A detected object is within 15–34 cm. Red alert zone.';tone='danger';symbol='!';}
    else if(min<55){headline='Approaching obstacle';message='An object is within 35–54 cm. Yellow caution zone.';tone='warn';symbol='!';}
    else if(humanFound){headline='Motion detected';message='A PIR sensor has detected movement near the vehicle.';tone='warn';symbol='!';}
    $('alertBox').className='panel alert-box'+(tone?' '+tone:'');$('alertSymbol').textContent=symbol;$('alertHeadline').textContent=headline;$('alertDescription').textContent=message;
    if(headline!==lastAlert){lastAlert=headline;log((mode==='demo'?'[Demo] ':'')+headline);}
  }
  function parseData(payload){
    if(!payload||typeof payload!=='object'||!('distF'in payload)||!('maxSpeed'in payload))return;
    telemetry={...telemetry,...payload};remoteControl=payload.remoteControl===true;
    lastTelemetry=Date.now();$('lastUpdate').textContent='Last update — '+stamp();updateControls();render();
  }
  function clearConnection(){wantsConnection=false;clearTimeout(reconnectTimer);if(socket){let old=socket;socket=null;old.onclose=null;old.onmessage=null;old.onerror=null;try{old.close()}catch(e){}}}
  function startDemo(){clearConnection();remoteControl=false;demoStep=0;telemetry={systemEnabled:false,maxSpeed:8,speedLimit:15,distF:85,distR:120,distL:64,distRefR:95,humanF:0,humanR:0,humanL:0,humanRefR:0};setMode('demo','disconnected');log('Switched to simulation');render();}
  function connect(){
    const url=$('wsUrl').value.trim();
    if(!/^wss?:\/\/[a-zA-Z0-9.\-\[\]:]+(?:\/[^\s]*)?$/.test(url)){$('connectHelp').textContent='Enter a valid ws:// or wss:// WebSocket address.';return;}
    if(location.protocol==='https:'&&url.startsWith('ws://')){$('connectHelp').textContent='This HTTPS page may block ws:// connections. Open the ESP32 page directly at http://192.168.4.1 or use an HTTPS/WSS bridge.';return;}
    clearConnection();wantsConnection=true;remoteControl=false;lastTelemetry=0;telemetry={systemEnabled:false,maxSpeed:0,speedLimit:15,distF:-1,distR:-1,distL:-1,distRefR:-1,humanF:0,humanR:0,humanL:0,humanRefR:0};setMode('live','connecting');render();$('connectHelp').textContent='Connecting to '+url+' — connect your device to the same ESP32 Wi-Fi network.';
    openSocket();
  }
  function openSocket(){
    if(!wantsConnection)return;
    const url=$('wsUrl').value.trim();let candidate;
    try{candidate=new WebSocket(url);}catch(e){setMode('live','disconnected');$('connectHelp').textContent='Invalid connection: '+e.message;return;}
    socket=candidate;
    candidate.onopen=()=>{if(candidate!==socket)return;setMode('live','connected');$('connectHelp').textContent='Connected to '+url+'. Live telemetry is updating automatically.';log('Connected to ESP32 WebSocket');};
    candidate.onmessage=event=>{if(candidate!==socket)return;try{parseData(JSON.parse(event.data));}catch(e){console.warn('Invalid ESP32 telemetry:',e)}};
    candidate.onerror=()=>{if(candidate===socket)$('connectHelp').textContent='Unable to reach ESP32. Check its power, Wi-Fi and WebSocket address.';};
    candidate.onclose=()=>{if(candidate!==socket)return;socket=null;remoteControl=false;setMode('live','disconnected');telemetry={...telemetry,maxSpeed:0,distF:-1,distR:-1,distL:-1,distRefR:-1,humanF:0,humanR:0,humanL:0,humanRefR:0};render();log('ESP32 connection lost — retrying');if(wantsConnection)reconnectTimer=setTimeout(openSocket,2500);};
  }
  function changeLimit(delta,reset){
    const current=Number(telemetry.speedLimit)||15;let next=reset?15:Math.max(5,Math.min(100,current+delta));
    if(mode==='demo'){telemetry.speedLimit=next;render();return;}
    if(socket&&socket.readyState===WebSocket.OPEN&&remoteControl){socket.send(JSON.stringify({cmd:'setSpeedLimit',value:next}));}
  }
  $('connectBtn').addEventListener('click',connect);
  $('demoBtn').addEventListener('click',startDemo);
  $('toConnection').addEventListener('click',()=>$('connectionSettings').scrollIntoView({behavior:'smooth',block:'center'}));
  $('limitMinus').addEventListener('click',()=>changeLimit(-1,false));$('limitPlus').addEventListener('click',()=>changeLimit(1,false));$('limitReset').addEventListener('click',()=>changeLimit(0,true));
  setInterval(()=>{
    if(mode==='demo'){
      demoStep++;
      const t=demoStep;
      telemetry.maxSpeed=Math.max(0,Math.round(8+6*Math.sin(t*.36)));
      telemetry.distF=Math.round(85-60*Math.max(0,Math.sin(t*.3)));
      telemetry.distR=Math.round(122+12*Math.sin(t*.2));
      telemetry.distL=Math.round(65+17*Math.sin(t*.25));
      telemetry.distRefR=Math.round(94+28*Math.cos(t*.21));
      telemetry.humanF=t%19===7||t%19===8?1:0;
      $('lastUpdate').textContent='Last update — demo '+stamp();render();
    }else if(lastTelemetry&&Date.now()-lastTelemetry>3500){
      if($('connectionText').textContent==='DEVICE CONNECTED'){
        $('lastUpdate').textContent='No recent telemetry received';
        telemetry={...telemetry,maxSpeed:0,distF:-1,distR:-1,distL:-1,distRefR:-1,humanF:0,humanR:0,humanL:0,humanRefR:0};render();
        // The WebSocket can remain open while the ESP32 has stopped sending telemetry.
      }
    }
  },1000);
  setMode('demo','disconnected');render();
  // Auto-connect when served by ESP32 or any non-local plain-HTTP host.
  if(location.protocol==='http:'&&location.hostname&&location.hostname!=='localhost'&&location.hostname!=='127.0.0.1') connect();
})();
</script>
</body>
</html>

)PARKSENSE_HTML";


#if CLOUD_UPLINK_ENABLED
// Network I/O in a separate FreeRTOS task, never in the main alert-control loop.
void cloudTelemetryTask(void *ignored) {
  // WSS streaming: one secure connection, updates every 350ms.
  // HTTPS POST: compatibility fallback when WSS is unavailable.
  // Network I/O stays on Core 0; local GPIO / ESP-NOW control in loop() remains independent.
  String lastAppliedCommandId;
  configTime(0, 0, "pool.ntp.org", "time.google.com");

  WebSocketsClient deviceWss;
  const String authHeader=String("Authorization: Bearer ") + CLOUD_DEVICE_TOKEN;
  bool wssStarted=false;
  unsigned long lastStreamSent=0, streamCount=0;
  unsigned long wifiConnectedAt=0, lastWifiAttempt=0;
  bool wasConnected=false;
  WiFiClientSecure secureClient;
  secureClient.setCACert(CLOUD_ROOT_CA);
  secureClient.setTimeout(4);
  HTTPClient http;
  http.setTimeout(3500);
  http.setConnectTimeout(3500);
  http.setReuse(true);
  bool httpReady=false;
  unsigned long lastFallbackUpload=0;
  int lastFallbackCode=0;

  deviceWss.onEvent([&](WStype_t type, uint8_t *payload, size_t length) {
    if(type==WStype_CONNECTED) {
      Serial.printf("[Cloud] WSS LIVE | CH=%d | RSSI=%d dBm\\n",
                    (int)currentWifiChannel, WiFi.RSSI());
    }else if(type==WStype_DISCONNECTED) {
      Serial.println("[Cloud] WSS disconnected; HTTPS fallback available");
    }else if(type==WStype_TEXT && length > 0 && length <= 256) {
      JsonDocument reply;
      if(deserializeJson(reply,payload,length)) return;
      const char *typeName=reply["type"] | "";
      const char *id=reply["id"] | "";
      const int requested=reply["value"] | -1;
      if(strcmp(typeName,"set_speed_limit")==0 && strlen(id)==16 &&
         requested>=5 && requested<=100) {
        bool validId=true;
        for(int i=0;i<16;i++){
          if(!((id[i]>='0'&&id[i]<='9')||(id[i]>='a'&&id[i]<='f')))validId=false;
        }
        if(!validId)return;
        speedLimit=requested; // WARNING only: no throttle or brake actuation
        lastAppliedCommandId=id;
        Serial.printf("[Cloud] WSS warning threshold applied: %d km/h\\n", requested);
      }
    }
  });

  for (;;) {
    const bool connected=WiFi.status()==WL_CONNECTED;
    if(connected!=wasConnected) {
      if(connected) {
        wifiConnectedAt=millis();
        const int ch=WiFi.channel();
        currentWifiChannel=(ch>=1&&ch<=14)?ch:0;
        Serial.printf("[Cloud] Hotspot CH=%d RSSI=%d dBm\\n",
                      (int)currentWifiChannel,WiFi.RSSI());
      }else{
        currentWifiChannel=0;
        wifiConnectedAt=0;
        if(wssStarted){deviceWss.disconnect();wssStarted=false;}
        if(httpReady){http.end();httpReady=false;}
        secureClient.stop();
        Serial.println("[Cloud] Wi-Fi offline, physical warning logic unaffected");
      }
      wasConnected=connected;
    }
    if(!connected&&(lastWifiAttempt==0||millis()-lastWifiAttempt>=10000UL)){
      lastWifiAttempt=millis();
      WiFi.begin(CLOUD_WIFI_SSID,CLOUD_WIFI_PASSWORD,0); // follow phone hotspot channel
    }
    if(connected && currentWifiChannel>=1 && time(nullptr)>1700000000){
      if(!wssStarted){
        // CA-pinned HTTPS trust, no 'insecure' fallback. Cloudflare reverse
        // proxy must support WebSocket Upgrade for /ws/device.
        deviceWss.beginSslWithCA("parkingdashboard.my.id",443,"/ws/device",CLOUD_ROOT_CA);
        deviceWss.setExtraHeaders(authHeader.c_str());
        deviceWss.setReconnectInterval(5000);
        deviceWss.enableHeartbeat(15000,4500,2);
        wssStarted=true;
      }
      deviceWss.loop();
      if(deviceWss.isConnected()){
        if(httpReady){http.end();httpReady=false;secureClient.stop();}
        if(lastStreamSent==0||millis()-lastStreamSent>=350UL){
          JsonDocument doc;
          makeTelemetry(doc);
          doc["cloudControl"]=1;
          if(lastAppliedCommandId.length()==16)doc["commandApplied"]=lastAppliedCommandId;
          String packet;
          serializeJson(doc,packet);
          if(deviceWss.sendTXT(packet)){
            lastStreamSent=millis();
            streamCount++;
            if(streamCount%40==1)Serial.printf("[Cloud] WSS sent=%lu RSSI=%d CH=%d\\n",
                                            streamCount,WiFi.RSSI(),(int)currentWifiChannel);
          }
        }
      }else if(wifiConnectedAt && millis()-wifiConnectedAt>=8000UL &&
               (lastFallbackUpload==0||millis()-lastFallbackUpload>=
                 (lastFallbackCode==HTTP_CODE_OK?2200UL:3500UL))){
        // Only retry POST if WSS has not connected. Avoid parallel HTTPS
        // handshakes flooding the phone hotspot.
        JsonDocument doc;
        makeTelemetry(doc);
        doc["cloudControl"]=1;
        if(lastAppliedCommandId.length()==16)doc["commandApplied"]=lastAppliedCommandId;
        String packet;
        serializeJson(doc,packet);
        if(!httpReady){
          httpReady=http.begin(secureClient,CLOUD_API_URL);
          if(httpReady){
            http.addHeader("Authorization",String("Bearer ")+CLOUD_DEVICE_TOKEN);
            http.addHeader("Content-Type","application/json");
            http.setReuse(true);
          }
        }
        const unsigned long start=millis();
        const int status=httpReady?http.POST(packet):-1;
        if(status==HTTP_CODE_OK){
          const String response=http.getString();
          if(response.length()>0 && response.length()<=512){
            JsonDocument reply;
            if(!deserializeJson(reply,response)){
              JsonVariantConst cmd=reply["command"];
              const char *typeName=cmd["type"] | "";
              const char *id=cmd["id"] | "";
              const int requested=cmd["value"] | -1;
              if(strcmp(typeName,"set_speed_limit")==0 &&
                 strlen(id)==16 && requested>=5 && requested<=100){
                bool validId=true;
                for(int i=0;i<16;i++){
                  if(!((id[i]>='0'&&id[i]<='9')||(id[i]>='a'&&id[i]<='f')))validId=false;
                }
                if(validId){
                  speedLimit=requested;
                  lastAppliedCommandId=id;
                  Serial.printf("[Cloud] HTTPS warning threshold applied=%d\\n",requested);
                }
              }
            }
          }
        }
        Serial.printf("[Cloud] HTTPS FALLBACK=%d time=%lu ms RSSI=%d CH=%d\\n",
                      status,millis()-start,WiFi.RSSI(),(int)currentWifiChannel);
        if(status<0){
          http.end();httpReady=false;secureClient.stop();
        }
        lastFallbackCode=status;
        lastFallbackUpload=millis();
      }
    }
    vTaskDelay(pdMS_TO_TICKS(35));
  }
}
#endif

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(POWER_SWITCH_PIN, INPUT_PULLUP);
  allAlertsOff();
  checkPowerSwitch();
  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  lastClkState = digitalRead(ENC_CLK);
  lastSwState = digitalRead(ENC_SW);
  allAlertsOff();

  esp_wifi_stop();
  delay(1000);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("Monitor Dashboard", PARKSENSE_AP_PASSWORD, 1);
  esp_wifi_set_ps(WIFI_PS_NONE);
  WiFi.setSleep(false);
  esp_wifi_set_max_tx_power(78);
  delay(2000);
  // STA will automatically move this SoftAP onto the hotspot channel.
#if CLOUD_UPLINK_ENABLED
  WiFi.setAutoReconnect(false); // Only the low-priority uplink task retries, channel 1.
  if (cloudCredentialsValid()) Serial.println("[Cloud] Config ready. Auto hotspot channel; sensor pods must match.");
  else Serial.println("[Cloud] OFFLINE: fill 3 settings at top of main.cpp; local alerts still work.");
#endif
  if(esp_now_init()!=ESP_OK){Serial.println("ESP-NOW Init Failed");return;}
  esp_now_register_recv_cb(OnDataRecv);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){request->send_P(200,"text/html",index_html);});
  server.addHandler(&ws); // local dashboard: read-only telemetry
  server.begin();
  Serial.println("PARKSENSE ONLINE | AP: Monitor Dashboard | http://192.168.4.1");
  Serial.println("Encoder: speed limit 5-100; encoder press resets to 15 km/h");
  Serial.println("GPIO14 switch: GND=parking alerts ON; OPEN=all alert outputs OFF");
#if CLOUD_UPLINK_ENABLED
  // Start only after all 3 local config fields have been filled. No Wi-Fi
  // association attempts while example credentials are in this file.
  if (cloudCredentialsValid()) {
    BaseType_t ok=xTaskCreatePinnedToCore(cloudTelemetryTask,"ParkSenseCloud",16384,nullptr,1,nullptr,0);
    if(ok!=pdPASS) Serial.println("[Cloud] Background task could not start");
  }
#endif
}

void loop() {
  checkEncoder();
  checkPowerSwitch();
  while(Serial2.available()>0)gps.encode(Serial2.read());
  if(gps.speed.isUpdated() && gps.speed.isValid() && gps.location.isValid() && gps.satellites.value()>3){
    liveGpsSpeed=(int)(gps.speed.kmph()+0.5);
    lastGpsUpdateTime=millis();
  }
  if(lastGpsUpdateTime==0 || millis()-lastGpsUpdateTime>3000)liveGpsSpeed=0;
  ws.cleanupClients();
  updateHardwareAlerts();
  broadcastTelemetry();
  delay(30);
}

'use strict';
const {test}=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const path=require('node:path');
const root=path.resolve(__dirname,'..');
const html=fs.readFileSync(path.join(root,'public','index.html'),'utf8');
const server=fs.readFileSync(path.join(root,'server.js'),'utf8');
const firmware=fs.readFileSync(path.join(root,'firmware/esp32-main/src/main.cpp'),'utf8');
const setup=fs.readFileSync(path.join(root,'tools/setup_firmware.py'),'utf8');

test('Public gallery shows only source-hosted CC BY assets with clear credits',()=>{
  for(const element of ['licensedPane','licensedFrame','creator3DLink','licensedDisclaimer','selectedModelNote']){
    assert.ok(html.includes('id="'+element+'"'),'Missing '+element);
  }
  assert.match(html,/REAL 3D/);
  assert.match(html,/zairiq-123 · CC BY/);
  assert.match(html,/generation unverified/i);
  assert.match(html,/sketchfab\.com\/models/);
  assert.match(html,/https:\/\/sketchfab\.com\/3d-models\//);
  assert.doesNotMatch(html,/4c8a2edd636f478994f021b9279ea1e1/); // old unverified NoAI entry
});

test('Browser security policy isolates allowed external 3D frames',()=>{
  assert.match(server,/default-src 'none'/);
  assert.match(server,/frame-src https:\/\/sketchfab\.com/);
  assert.match(server,/script-src 'unsafe-inline'/);
  assert.match(server,/DASHBOARD_PASSWORD\.length < 16/);
  assert.match(server,/DEVICE_TOKEN\.length < 24/);
});

test('ESP32 cloud opt-in builds offline and only associates on channel 1',()=>{
  assert.match(firmware,/#if __has_include\("cloud_config\.h"\)/);
  assert.match(firmware,/#define CLOUD_UPLINK_ENABLED false/);
  assert.match(firmware,/WiFi\.begin\(CLOUD_WIFI_SSID, CLOUD_WIFI_PASSWORD, 1\)/);
  assert.match(firmware,/esp_wifi_set_channel\(1, WIFI_SECOND_CHAN_NONE\)/);
  assert.match(firmware,/xTaskCreatePinnedToCore\(cloudTelemetryTask/);
  assert.match(firmware,/updateHardwareAlerts\(\);/);
  assert.match(firmware,/broadcastTelemetry\(\);/);
  assert.doesNotMatch(firmware,/WiFi\.begin\(CLOUD_WIFI_SSID, CLOUD_WIFI_PASSWORD\);/);
});

test('One-command setup blocks unverified channels and keeps secrets local',()=>{
  assert.match(setup,/Type CH1/);
  assert.match(setup,/cloud_config\.h/);
  assert.match(setup,/getpass\.getpass/);
  assert.match(setup,/parkingdashboard\.my\.id\/api\/device\/telemetry/);
  assert.match(setup,/type FLASH/);
  const gitignore=fs.readFileSync(path.join(root,'.gitignore'),'utf8');
  assert.match(gitignore,/firmware\/esp32-main\/include\/cloud_config\.h/);
});

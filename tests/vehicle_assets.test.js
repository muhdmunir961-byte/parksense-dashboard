'use strict';
const {test}=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const path=require('node:path');
const root=path.resolve(__dirname,'..');
const html=fs.readFileSync(path.join(root,'public/index.html'),'utf8');
const server=fs.readFileSync(path.join(root,'server.js'),'utf8');
const firmware=fs.readFileSync(path.join(root,'firmware/esp32-main/src/main.cpp'),'utf8');
const setup=fs.readFileSync(path.join(root,'tools/setup_firmware.py'),'utf8');

test('Licensed Malaysian 3D gallery has explicit author credits and source links',()=>{
  for(const id of ['licensedPane','licensedFrame','creator3DLink','licensedDisclaimer','selectedModelNote'])
    assert.ok(html.includes('id="'+id+'"'),'Missing '+id);
  const models=[
    '4f62bfca654643c3858a7688e7a708ed', // Myvi
    'e34de48e167243d997719dfe293efbad', // Saga 1985 light
    '7e8adb5d0d2e4dfca405e07567835406', // Saga 1985 HD
    '77d780d5f22e49239ccb15b49d269025', // Proton X50
    '92443d2ccf2e44a5bbf7320d3dfe38ae', // Proton Waja
    '65f15580b9d848ab8570347e6df861fa', // Proton Jumbuck
    'b3306f377c694fb0925996411bd1c6d5', // Honda Civic PDRM
  ];
  for(const id of models) assert.ok(html.includes(id), 'Missing Sketchfab source '+id);
  assert.match(html,/CC BY/);
  assert.match(html,/generation unverified/i);
  assert.doesNotMatch(html,/4c8a2edd636f478994f021b9279ea1e1/);
});

test('Security policy restricts external 3D frames and remote control uses verified device token',()=>{
  assert.match(server,/default-src 'none'/);
  assert.match(server,/frame-src https:\/\/sketchfab\.com/);
  assert.match(server,/DASHBOARD_PASSWORD\.length < 16/);
  assert.match(server,/DEVICE_TOKEN\.length < 24/);
  assert.match(server,/if\(!authenticated\(req\)\)/);
  assert.match(server,/if\(!sameOrigin\(req\)\)/);
  assert.match(server,/commandApplied===pendingLimitCommand\.id/);
  assert.match(server,/const COMMAND_TTL_MS = 12000/);
});

test('ESP32 remote setting applies only warning threshold with 5-100 km/h validation',()=>{
  assert.match(firmware,/#define CLOUD_UPLINK_ENABLED true/);
  assert.match(firmware,/#define CLOUD_WIFI_SSID "ISI_NAMA_HOTSPOT_WIFI_2_4GHZ"/);
  assert.match(firmware,/#define CLOUD_WIFI_PASSWORD "ISI_PASSWORD_WIFI"/);
  assert.match(firmware,/#define CLOUD_DEVICE_TOKEN "ISI_DEVICE_TOKEN_COOLIFY_MIN_24"/);
  assert.match(firmware,/bool cloudCredentialsValid\(\)/);
  assert.match(firmware,/if \(cloudCredentialsValid\(\)\)/);
  assert.doesNotMatch(firmware,/#include "cloud_config\.h"/);
  assert.match(firmware,/WiFi\.begin\(CLOUD_WIFI_SSID, CLOUD_WIFI_PASSWORD, 0\)/);
  assert.match(firmware,/doc\["wifiChannel"\] = \(int\)currentWifiChannel/);
  assert.match(firmware,/http\.setReuse\(true\)/);
  assert.match(firmware,/secureClient\.setCACert\(CLOUD_ROOT_CA\)/);
  assert.doesNotMatch(firmware,/esp_wifi_set_channel\(1, WIFI_SECOND_CHAN_NONE\)/);
  assert.match(firmware,/doc\["cloudControl"\] = 1/);
  assert.match(firmware,/strcmp\(type,\s*"set_speed_limit"\)\s*==\s*0/);
  assert.match(firmware,/requested\s*>=\s*5 && requested\s*<=\s*100/);
  assert.match(firmware,/speedLimit\s*=\s*requested/);
  assert.match(firmware,/lastAppliedCommandId\s*=\s*id/);
  assert.match(firmware,/updateHardwareAlerts\(\);/);
  assert.doesNotMatch(firmware,/setInsecure\(/);
  assert.doesNotMatch(firmware,/WiFi\.begin\(CLOUD_WIFI_SSID, CLOUD_WIFI_PASSWORD\);/);
});

test('One-file ESP32 configuration is safe and the old wizard never writes unused secrets',()=>{
  assert.match(firmware,/#define CLOUD_UPLINK_ENABLED true/);
  assert.match(firmware,/#define CLOUD_WIFI_SSID "ISI_NAMA_HOTSPOT_WIFI_2_4GHZ"/);
  assert.match(firmware,/#define CLOUD_WIFI_PASSWORD "ISI_PASSWORD_WIFI"/);
  assert.match(firmware,/#define CLOUD_DEVICE_TOKEN "ISI_DEVICE_TOKEN_COOLIFY_MIN_24"/);
  assert.match(firmware,/bool cloudCredentialsValid\(\)/);
  assert.match(firmware,/if \(cloudCredentialsValid\(\)\)/);
  assert.match(firmware,/PARKSENSE_AP_PASSWORD/);
  assert.match(firmware,/https:\/\/parkingdashboard\.my\.id\/api\/device\/telemetry/);
  assert.match(setup,/No files were changed/);
  assert.doesNotMatch(setup,/write_text\(/);
  assert.doesNotMatch(firmware,/#include "cloud_config\.h"/);
});

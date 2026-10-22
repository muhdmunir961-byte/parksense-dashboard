'use strict';
const {test}=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const vm=require('node:vm');
const path=require('node:path');
const html=fs.readFileSync(path.join(__dirname,'../public/index.html'),'utf8');
const source=html.match(/<script>([\s\S]*?)<\/script>/)?.[1]||'';
function part(start,end){
  const a=source.indexOf(start),b=source.indexOf(end,a+start.length);
  assert.ok(a>=0&&b>a,'Missing section '+start);
  return source.slice(a,b);
}
const catalogSrc=part('const vehicleCatalog=','// Hand-crafted starting geometry');
const catalog=new Function(catalogSrc+'return vehicleCatalog')();
const metaSrc=part('  const reference3DModels={','  let realRef=');
const entries=new Function(metaSrc+'return {reference3DModels,real3DChoices}')();

test('Dashboard JS parses, IDs are unique, every referenced element exists',()=>{
  assert.doesNotThrow(()=>new vm.Script(source));
  const ids=[...html.matchAll(/\bid="([^"]+)"/g)].map(x=>x[1]);
  assert.equal(new Set(ids).size,ids.length);
  for(const [,id] of source.matchAll(/\$\('([^']+)'\)/g))assert.ok(ids.includes(id),'Missing ID '+id);
});

test('Exactly seven verified creator-hosted models; nothing without source is selectable',()=>{
  const buttons=[...html.matchAll(/data-realmodel="([^"]+)"/g)].map(x=>x[1]);
  assert.deepEqual(buttons,['myvi','saga-lite','saga-hd','x50','waja','jumbuck','civic']);
  const choices=entries.real3DChoices;
  assert.deepEqual(Object.keys(choices),buttons);
  for(const [key,[brand,model]] of Object.entries(choices)){
    assert.ok(catalog[brand]?.[model],'Unknown selectable car '+brand+' '+model);
    const asset=entries.reference3DModels[key];
    assert.ok(asset?.embed.includes('/embed?'),key+' missing embed');
    assert.ok(asset.source.startsWith('https://sketchfab.com/3d-models/'));
    assert.ok(/CC BY/.test(asset.label),'No author attribution '+key);
    assert.ok(asset.accuracy.length>40,'Missing identification limitations '+key);
  }
  assert.deepEqual(Object.keys(catalog),['Perodua','Proton','Honda']);
  assert.equal(Object.values(catalog).reduce((sum,models)=>sum+Object.keys(models).length,0),7);
  assert.match(html,/class="preview-actions" hidden/);
  assert.match(html,/class="vehicle-selectors" hidden/);
});

test('View opens only creator 3D, never mutates ESP32 telemetry or alarm state',()=>{
  assert.match(source,/setVehicleView\('licensed'\)/);
  assert.match(source,/creatorKeyFor\(vehicleMeta\?\.brand,vehicleMeta\?\.model\)/);
  assert.match(source,/if\(view!=='licensed'&&\$\('licensedFrame'\)\.getAttribute\('src'\)\)/);
  assert.match(source,/parseData\(msg\.data\)/);
  assert.match(source,/hardware alerts unaffected|hardware alarms remain active|hardware LED and buzzer/i);
});

test('Speed setting UI controls the warning THRESHOLD, never physical speed',()=>{
  for(const id of ['limitTarget','limitApply','limitMinus','limitPlus','limitReset','limitFeedback'])
    assert.ok(html.includes('id="'+id+'"'),id);
  assert.match(html,/TARGET ALERT LIMIT/);
  assert.match(source,/\/api\/control\/speed-limit/);
  assert.match(source,/payload\.remoteControl===true/);
  assert.match(source,/ESP32 CONFIRMED/);
  assert.match(source,/Number\.isInteger\(value\)\|\|value<5\|\|value>100/);
});

test('New phone hotspot firmware channel is rendered separately from ESP-NOW pod health',()=>{
  for(const id of ['radioHealth','radioHealthTitle','radioHealthDetail','radioChannelBadge','cloudWifiChannel','cloudPodCount','liveWifiChannel']) {
    assert.match(html,new RegExp('id="'+id+'"'));
  }
  assert.match(source,/function updateRadioHealth\(active\)/);
  assert.match(source,/updateRadioHealth\(active\)/);
  assert.match(source,/Number\.isInteger\(telemetry\.wifiChannel\)/);
  assert.match(source,/cannot automatically follow a different hotspot channel/);
  assert.match(source,/Main ESP32 uses CH /);
  assert.match(source,/Demo sensor values/);
  assert.match(source,/Older main\.cpp builds may omit wifiChannel/);
  assert.match(source,/state\.wifiChannel/);
  assert.match(source,/state\.activePods/);
});

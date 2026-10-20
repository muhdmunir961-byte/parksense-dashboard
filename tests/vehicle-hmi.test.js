'use strict';
const {test}=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const vm=require('node:vm');
const path=require('node:path');
const html=fs.readFileSync(path.join(__dirname,'../public/index.html'),'utf8');
const match=html.match(/<script>([\s\S]*?)<\/script>/);
const source=match?.[1]||'';
function extract(first,last){
  const a=source.indexOf(first),b=source.indexOf(last,a+1);
  assert.ok(a!==-1&&b>a,'Missing client section '+first);
  return source.slice(a,b);
}
const catalogSource=extract('const vehicleCatalog=','// Hand-crafted starting geometry');
const catalog=new Function(catalogSource+';return vehicleCatalog;')();

test('Dashboard script parses and all DOM references exist',()=>{
  assert.doesNotThrow(()=>new vm.Script(source));
  const ids=[...html.matchAll(/\bid="([^"]+)"/g)].map(x=>x[1]);
  assert.equal(new Set(ids).size,ids.length);
  const directRefs=[...source.matchAll(/\$\('([^']+)'\)/g)].map(m=>m[1]);
  for(const id of directRefs)assert.ok(ids.includes(id),'Missing element '+id);
});

test('Only two verified 3D cars can be selected',()=>{
  assert.deepEqual(Object.keys(catalog),['Perodua','Proton']);
  assert.deepEqual(Object.keys(catalog.Perodua),['Myvi']);
  assert.deepEqual(Object.keys(catalog.Proton),['Saga 1985']);
  assert.deepEqual(catalog.Perodua.Myvi[1],[['Generation unverified','myvi3']]);
  assert.deepEqual(catalog.Proton['Saga 1985'][1],[['1985 · First generation','saga1']]);
  const buttons=[...html.matchAll(/data-realmodel="([^"]+)"/g)].map(m=>m[1]);
  assert.deepEqual(buttons,['myvi','saga']);
  assert.match(html,/class="preview-actions" hidden/);
  assert.match(html,/class="vehicle-selectors" hidden/);
  assert.match(html,/class="paint-options"[^>]*hidden/);
});

test('Real 3D gallery uses author links and does not claim Myvi generation',()=>{
  assert.match(source,/4f62bfca654643c3858a7688e7a708ed/);
  assert.match(source,/7e8adb5d0d2e4dfca405e07567835406/);
  assert.match(source,/generation unverified/i);
  assert.match(source,/zairiq-123/);
  assert.match(source,/setVehicleView\('licensed'\)/);
  assert.match(source,/selectedGen=isMyvi\?'Generation unverified':'1985 · First generation'/);
});

test('Only browser visuals change, safety telemetry and physical warnings remain read only',()=>{
  for(const field of ['distF','distR','distL','distRefR','humanF','humanR','humanL','humanRefR']){
    assert.ok(source.includes(field),'Missing telemetry key '+field);
  }
  assert.match(source,/parseData\(msg\.data\)/);
  assert.match(source,/hardware alerts unaffected|hardware alarms remain active|hardware LED and buzzer/i);
});

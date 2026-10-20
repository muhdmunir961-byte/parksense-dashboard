'use strict';
const { test } = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const path = require('node:path');
const root=path.resolve(__dirname,'..');
const html=fs.readFileSync(path.join(root,'public','index.html'),'utf8');
const server=fs.readFileSync(path.join(root,'server.js'),'utf8');

test('ParkSense inline dashboard script parses and DOM ids remain consistent',()=>{
  const match=html.match(/<script>([\s\S]*?)<\/script>/);
  assert.ok(match,'Dashboard inline script exists');
  assert.doesNotThrow(()=>new vm.Script(match[1],{filename:'index.dashboard.js'}));
  const ids=[...html.matchAll(/\bid="([^"]+)"/g)].map(m=>m[1]);
  assert.equal(new Set(ids).size,ids.length,'All HTML element ids should be unique');
  const references=[...match[1].matchAll(/\$\('([^']+)'\)/g)].map(m=>m[1]);
  for(const id of references)assert.ok(ids.includes(id),'Missing dashboard element id: '+id);
});
test('Real-world media references have explicit credit controls and only load by user choice',()=>{
  for(const id of ['referenceThumb','referenceLink','realPhoto','realPhotoLink','photoPane','viewPhoto','viewLicensed','licensedPane','licensedFrame','creator3DLink']){
    assert.match(html,new RegExp('id="'+id+'"'),'missing '+id);
  }
  assert.match(html,/commons.wikimedia.org/);
  assert.match(html,/CC BY-SA 4\.0/);
  assert.match(html,/NoAI \(embed only\)/);
  assert.match(html,/if\(view==='licensed'/);
  assert.match(html,/removeAttribute\('src'\)/);
});
test('Vehicle profile references include genuine generation-specific sources',()=>{
  for(const profile of ['myvi1','myvi2','myvi3','myvi3f','saga1'])
    assert.match(html,new RegExp('\\b'+profile+':\\['),'missing verified profile '+profile);
  assert.match(html,/2018 Myvi Advance\.jpg/);
  assert.match(html,/Proton Saga 1985\.jpg/);
});
test('CSP only grants required remote origin permissions',()=>{
  assert.match(server,/img-src 'self' data: https:\/\/commons\.wikimedia\.org https:\/\/upload\.wikimedia\.org/);
  assert.match(server,/frame-src https:\/\/sketchfab\.com/);
  assert.match(server,/default-src 'none'/);
});

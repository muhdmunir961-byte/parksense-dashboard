'use strict';
const { test } = require('node:test');
const assert = require('node:assert/strict');
const { spawn } = require('node:child_process');
const { WebSocket } = require('ws');

const port = 30000 + Math.floor(Math.random()*18000);
const host = `http://127.0.0.1:${port}`;
const pw = 'test-secret-long-enough-123456789';
const token = 'test-device-token-long-enough-1234567890';
const child = spawn(process.execPath, ['server.js'], {
  cwd: require('node:path').join(__dirname,'..'),
  env: {...process.env, PORT:String(port), DASHBOARD_PASSWORD:pw, DEVICE_TOKEN:token},
  stdio:'ignore'
});
async function ready() {
  for(let i=0;i<60;i++){
    try{const res=await fetch(host+'/health');if(res.ok)return;}catch{}
    await new Promise(r=>setTimeout(r,100));
  }
  throw Error('Server did not start');
}
process.once('exit',()=>{child.kill()});

test('Auth, protected telemetry and browser WebSocket', async (t)=>{
  t.after(()=>child.kill());
  await ready();
  assert.equal((await fetch(host+'/')).status,200); // fetch follows /login
  const invalid=await fetch(host+'/api/login',{method:'POST',headers:{'content-type':'application/json'},body:JSON.stringify({password:'bad'})});
  assert.equal(invalid.status,401);
  const login=await fetch(host+'/api/login',{method:'POST',headers:{'content-type':'application/json'},body:JSON.stringify({password:pw})});
  assert.equal(login.status,200);
  const cookie=login.headers.get('set-cookie').split(';')[0];
  const unauthorized=await fetch(host+'/api/device/status');assert.equal(unauthorized.status,401);
  const authorized=await fetch(host+'/api/device/status',{headers:{cookie}});assert.equal(authorized.status,200);
  assert.equal((await authorized.json()).deviceOnline,false);
  const data={systemEnabled:true,maxSpeed:8,speedLimit:15,distF:85,distR:120,distL:64,distRefR:95,humanF:0,humanR:0,humanL:0,humanRefR:0};
  const headers={'content-type':'application/json',authorization:'Bearer '+token};
  const wrong=await fetch(host+'/api/device/telemetry',{method:'POST',headers:{...headers,authorization:'Bearer BAD'},body:JSON.stringify(data)});
  assert.equal(wrong.status,401);
  const sent=await fetch(host+'/api/device/telemetry',{method:'POST',headers,body:JSON.stringify(data)});assert.equal(sent.status,200);
  assert.equal((await sent.json()).command,null);
  const ws = new WebSocket(`ws://127.0.0.1:${port}/ws`,{headers:{cookie,origin:host}});
  await new Promise((resolve,reject)=>{ws.once('open',resolve);ws.once('error',reject)});
  t.after(()=>ws.close());
  const missingMode=await fetch(host+'/api/device/telemetry',{method:'POST',headers,body:JSON.stringify({...data,systemEnabled:'maybe'})});
  assert.equal(missingMode.status,422);
  const next=await fetch(host+'/api/device/telemetry',{method:'POST',headers,body:JSON.stringify({...data,systemEnabled:false})});
  assert.deepEqual((await next.json()).command,null);
});

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
  // Optional hotspot radio channel survives authenticated device -> server -> WSS.
  const channelSeen=new Promise((resolve,reject)=>{
    const timeout=setTimeout(()=>{ws.off('message',listen);reject(Error('Channel missing in live WSS'))},1800);
    function listen(raw) {
      let msg;
      try { msg=JSON.parse(String(raw)); } catch { return; }
      if(msg.type==='telemetry' && msg.data.wifiChannel===6) {
        clearTimeout(timeout);ws.off('message',listen);resolve();
      }
    }
    ws.on('message',listen);
  });
  const channelPost=await fetch(host+'/api/device/telemetry',{
    method:'POST',headers,body:JSON.stringify({...data,wifiChannel:6,distL:-1})
  });
  assert.equal(channelPost.status,200);
  const channelStatus=await fetch(host+'/api/device/status',{headers:{cookie}});
  assert.equal(channelStatus.status,200);
  const channelData=await channelStatus.json();
  assert.equal(channelData.deviceOnline,true);
  assert.equal(channelData.wifiChannel,6);
  assert.equal(channelData.activePods,3);
  await channelSeen;
  for(const badChannel of [0,15,-1,1.5,'6',null]){
    const rejected=await fetch(host+'/api/device/telemetry',{
      method:'POST',headers,body:JSON.stringify({...data,wifiChannel:badChannel})
    });
    assert.equal(rejected.status,422,'Invalid channel should be rejected: '+badChannel);
  }
  const missingMode=await fetch(host+'/api/device/telemetry',{method:'POST',headers,body:JSON.stringify({...data,systemEnabled:'maybe'})});
  assert.equal(missingMode.status,422);
  const next=await fetch(host+'/api/device/telemetry',{method:'POST',headers,body:JSON.stringify({...data,systemEnabled:false})});
  assert.deepEqual((await next.json()).command,null);

  // Speed-warning threshold control: unauthenticated requests, cross-site pages,
  // unsupported firmware and malformed values may not modify ESP32 state.
  const endpoint=host+'/api/control/speed-limit';
  const safeOrigin={cookie,origin:host,'content-type':'application/json'};
  const postLimit=(value,headers=safeOrigin)=>fetch(endpoint,{
    method:'POST',headers,body:JSON.stringify({value})
  });
  assert.equal((await postLimit(20,{'content-type':'application/json',origin:host})).status,401);
  assert.equal((await postLimit(20,{...safeOrigin,origin:'https://attacker.invalid'})).status,403);
  assert.equal((await postLimit(20,{cookie,origin:host,'content-type':'text/plain'})).status,415);
  assert.equal((await postLimit(20)).status,409); // old firmware has no cloudControl capability

  // Updated main ESP32 advertises support on its authenticated telemetry stream.
  const fresh={...data,cloudControl:1};
  const upgrade=await fetch(host+'/api/device/telemetry',{method:'POST',headers,body:JSON.stringify(fresh)});
  assert.equal(upgrade.status,200);
  assert.equal((await upgrade.json()).command,null);
  assert.equal((await postLimit(0)).status,422);
  assert.equal((await postLimit(101)).status,422);
  assert.equal((await postLimit(12.5)).status,422);
  assert.equal((await postLimit('20')).status,422);
  const observed=[];
  ws.on('message',raw=>{try{observed.push(JSON.parse(String(raw)))}catch{}});
  const requested=await postLimit(25);
  assert.equal(requested.status,202);
  const accepted=await requested.json();
  assert.equal(accepted.state,'pending');
  assert.match(accepted.commandId,/^[a-f0-9]{16}$/);
  assert.equal((await postLimit(30)).status,409); // one outstanding command maximum

  let telemetryResponse=await fetch(host+'/api/device/telemetry',{
    method:'POST',headers,body:JSON.stringify(fresh)
  });
  let returned=await telemetryResponse.json();
  assert.deepEqual(returned.command,{type:'set_speed_limit',id:accepted.commandId,value:25});

  // A false acknowledgement must never be treated as applied.
  telemetryResponse=await fetch(host+'/api/device/telemetry',{
    method:'POST',headers,
    body:JSON.stringify({...fresh,commandApplied:accepted.commandId,speedLimit:24})
  });
  returned=await telemetryResponse.json();
  assert.equal(returned.command.id,accepted.commandId);

  // The ESP32's next POST proves that its actual threshold now equals 25 km/h.
  telemetryResponse=await fetch(host+'/api/device/telemetry',{
    method:'POST',headers,
    body:JSON.stringify({...fresh,commandApplied:accepted.commandId,speedLimit:25})
  });
  returned=await telemetryResponse.json();
  assert.equal(returned.command,null);
  await new Promise((resolve,reject)=>{
    const start=Date.now();
    const timer=setInterval(()=>{
      if(observed.some(msg=>msg.type==='speed_limit_command'&&msg.state==='applied'
         &&msg.id===accepted.commandId)){clearInterval(timer);resolve();}
      else if(Date.now()-start>2000){clearInterval(timer);reject(Error('Missing ESP32-applied WebSocket acknowledgement'));}
    },20);
  });
});

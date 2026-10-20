'use strict';
const http = require('node:http');
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const { WebSocketServer, WebSocket } = require('ws');

const PORT = Number(process.env.PORT || 3000);
const DASHBOARD_PASSWORD = process.env.DASHBOARD_PASSWORD || '';
const DEVICE_TOKEN = process.env.DEVICE_TOKEN || '';
if (DASHBOARD_PASSWORD.length < 16 || DEVICE_TOKEN.length < 24 || DASHBOARD_PASSWORD === DEVICE_TOKEN) {
  console.error('Set independent strong DASHBOARD_PASSWORD (16+ chars) and DEVICE_TOKEN (24+ chars) environment variables.');
  process.exit(1);
}
const sessionKey = crypto.createHash('sha256').update(DASHBOARD_PASSWORD + '\0' + DEVICE_TOKEN).digest();
const clients = new Set();
const wss = new WebSocketServer({ noServer: true, maxPayload: 1024, perMessageDeflate: false });
let lastTelemetry = null;
let lastSeenAt = 0;
let lastOnlineState = false;
const ONLINE_WINDOW_MS = 5000;
const COMMAND_TTL_MS = 12000; // In-memory, not persistent. Never apply stale instructions.
const loginAttempts = new Map();
let pendingLimitCommand = null;
let lastLimitCommandTime = 0;
function announceCommand(state, command, message='') {
  broadcast({type:'speed_limit_command',state,id:command?.id||null,value:command?.value||null,message});
}
function expireCommand() {
  if (pendingLimitCommand && (Date.now()>pendingLimitCommand.expiresAt || !isOnline())) {
    const old = pendingLimitCommand;
    pendingLimitCommand = null;
    announceCommand('failed',old,'ESP32 did not acknowledge within the allowed time or disconnected.');
  }
}
function sameOrigin(req) {
  const origin = req.headers.origin;
  if (typeof origin !== 'string') return false; // Browser-only control, blocks simple CSRF.
  try {
    const u=new URL(origin);
    return (u.protocol==='https:'||u.protocol==='http:') && u.host===req.headers.host
      && u.protocol===(secureCookie(req)?'https:':'http:');
  }catch{return false;}
}

function safeEqual(a, b) {
  const aa = Buffer.from(String(a)), bb = Buffer.from(String(b));
  return aa.length === bb.length && crypto.timingSafeEqual(aa, bb);
}
function getCookie(req, key) {
  const header = String(req.headers.cookie || '');
  const item = header.split(';').map(x => x.trim()).find(x => x.startsWith(key + '='));
  return item ? item.slice(key.length + 1) : '';
}
function sign(value) { return crypto.createHmac('sha256', sessionKey).update(value).digest('hex'); }
function authenticated(req) {
  const token = getCookie(req, 'ps_session');
  const split = token.indexOf('.');
  if (split < 0) return false;
  const expires = token.slice(0, split), mac = token.slice(split + 1);
  const exp = Number(expires);
  return /^\d{13}$/.test(expires) && exp > Date.now() && exp < Date.now() + 86401e3 && safeEqual(mac, sign(expires));
}
function respond(res, status, text, type='text/plain; charset=utf-8', headers={}) {
  res.writeHead(status, { 'Content-Type': type, 'Cache-Control': 'no-store', 'X-Content-Type-Options': 'nosniff', 'Referrer-Policy': 'no-referrer', 'X-Frame-Options': 'DENY', ...headers });
  res.end(text);
}
function json(res, status, data, headers={}) { respond(res, status, JSON.stringify(data), 'application/json; charset=utf-8', headers); }
function redirect(res, url) { res.writeHead(302, { Location: url, 'Cache-Control':'no-store' });res.end(); }
function publicFile(res, filename) {
  respond(res,200,fs.readFileSync(path.join(__dirname, 'public',filename)), 'text/html; charset=utf-8', {
    'Content-Security-Policy': "default-src 'none'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; connect-src 'self' ws: wss:; img-src 'self' data: https://commons.wikimedia.org https://upload.wikimedia.org; frame-src https://sketchfab.com; base-uri 'none'; form-action 'self'"
  });
}
async function body(req, maxBytes=4096) {
  const chunks=[];let total=0;
  for await (const chunk of req) {
    total+=chunk.length;
    if (total>maxBytes) { const e=new Error('Request too large');e.status=413;throw e; }
    chunks.push(chunk);
  }
  return Buffer.concat(chunks).toString('utf8');
}
function isOnline(){ return lastTelemetry!==null && Date.now()-lastSeenAt < ONLINE_WINDOW_MS; }
function broadcast(object){ const data=JSON.stringify(object);for(const client of clients)if(client.readyState===WebSocket.OPEN)client.send(data); }
function normNumber(data,key,min,max){const v=Number(data[key]);if(!Number.isFinite(v)||!Number.isInteger(v)||v<min||v>max)return null;return v;}
function normalise(data){
  if (!data || typeof data!=='object'||Array.isArray(data))return null;
  const result={};
  for(const [key,min,max] of [['maxSpeed',0,250],['speedLimit',5,100],['distF',-1,400],['distR',-1,400],['distL',-1,400],['distRefR',-1,400]]){
    let n=normNumber(data,key,min,max);
    if(n===null || (key.startsWith('dist') && n!==-1 && n<2))return null;
    result[key]=n;
  }
  for(const key of ['humanF','humanR','humanL','humanRefR']){
    const n=normNumber(data,key,0,1);if(n===null)return null;result[key]=n;
  }
  if (typeof data.systemEnabled !== 'boolean') return null;
  result.systemEnabled = data.systemEnabled;
  // Command capability must be explicitly advertised by updated ESP32 firmware.
  result.remoteControl = data.cloudControl === 1;
  if (typeof data.commandApplied === 'string' && /^[a-f0-9]{16}$/.test(data.commandApplied)) {
    result.commandApplied = data.commandApplied;
  }
  return result;
}
function secureCookie(req){return req.socket.encrypted || req.headers['x-forwarded-proto']==='https';}
const server=http.createServer(async(req,res)=>{
  const url = new URL(req.url || '/', 'http://localhost');
  const route=url.pathname;
  try {
    if(req.method==='GET' && route==='/health'){return json(res,200,{status:'ok'});}
    if(req.method==='GET' && route==='/login'){
      if(authenticated(req))return redirect(res,'/');
      return publicFile(res,'login.html');
    }
    if(req.method==='POST' && route==='/api/login'){
      const remote=req.socket.remoteAddress || 'unknown';
      const info=loginAttempts.get(remote) || {count:0,until:0};
      if(info.until > Date.now())return json(res,429,{error:'Too many attempts. Try again later.'});
      const parsed=JSON.parse(await body(req,1024));
      if(!safeEqual(parsed.password || '',DASHBOARD_PASSWORD)){
        info.count++;
        if(info.count>=8){info.until=Date.now()+60000;info.count=0;}
        loginAttempts.set(remote,info);
        return json(res,401,{error:'Incorrect password'});
      }
      loginAttempts.delete(remote);
      const expiry=String(Date.now()+86400e3);
      const token=expiry+'.'+sign(expiry);
      return json(res,200,{ok:true},{'Set-Cookie':`ps_session=${token}; Path=/; HttpOnly; SameSite=Strict; Max-Age=86400${secureCookie(req)?'; Secure':''}`});
    }
    if(req.method==='POST' && route==='/api/device/telemetry'){
      const header=String(req.headers.authorization || '');
      if(!header.startsWith('Bearer ')||!safeEqual(header.slice(7),DEVICE_TOKEN))return json(res,401,{error:'Unauthorized device'});
      const payload=normalise(JSON.parse(await body(req)));
      if(!payload)return json(res,422,{error:'Invalid telemetry'});
      const wasOnline=isOnline();
      lastTelemetry=payload;lastSeenAt=Date.now();
      if(!wasOnline)broadcast({type:'status',deviceOnline:true});
      expireCommand();
      // Acknowledgement is explicit: match command ID *and* the new physical limit.
      if(pendingLimitCommand && payload.commandApplied===pendingLimitCommand.id
         && payload.speedLimit===pendingLimitCommand.value) {
        const confirmed=pendingLimitCommand;
        pendingLimitCommand=null;
        announceCommand('applied',confirmed,'ESP32 confirmed the new alert threshold.');
      }
      broadcast({type:'telemetry',data:payload});
      // ESP32 polls naturally with telemetry HTTPS uploads; NO inbound device port required.
      const pending=pendingLimitCommand && payload.remoteControl && isOnline()
        ?{type:'set_speed_limit',id:pendingLimitCommand.id,value:pendingLimitCommand.value}
        :null;
      return json(res,200,{ok:true,command:pending});
    }
    if(req.method==='POST' && route==='/api/control/speed-limit'){
      if(!authenticated(req))return json(res,401,{error:'Login required'});
      if(!sameOrigin(req))return json(res,403,{error:'Same-origin request required'});
      if(!String(req.headers['content-type']||'').toLowerCase().startsWith('application/json'))
        return json(res,415,{error:'JSON body required'});
      expireCommand();
      if(!isOnline()||lastTelemetry.remoteControl!==true)
        return json(res,409,{error:'ESP32 not online or firmware does not support dashboard threshold controls'});
      if(pendingLimitCommand)
        return json(res,409,{error:'Previous threshold setting awaits ESP32 confirmation'});
      const value=JSON.parse(await body(req,256))?.value;
      if(typeof value!=='number'||!Number.isInteger(value)||value<5||value>100)
        return json(res,422,{error:'Alert threshold must be a whole number from 5 to 100 km/h'});
      // Keep HTTP command endpoint rate-limited; no high-frequency network flood.
      if(Date.now()-lastLimitCommandTime<300)
        return json(res,429,{error:'Please wait before changing the threshold again'});
      lastLimitCommandTime=Date.now();
      const command={id:crypto.randomBytes(8).toString('hex'),value,expiresAt:Date.now()+COMMAND_TTL_MS};
      pendingLimitCommand=command;
      announceCommand('pending',command,'Waiting for ESP32 to acknowledge setting');
      return json(res,202,{ok:true,state:'pending',commandId:command.id,value});
    }
    if(req.method==='GET' && route==='/api/device/status'){
      if(!authenticated(req))return json(res,401,{error:'Login required'});
      return json(res,200,{deviceOnline:isOnline(),updatedAt:lastSeenAt||null});
    }
    if(req.method==='POST' && route==='/api/logout'){
      return json(res,200,{ok:true},{'Set-Cookie':'ps_session=; Path=/; Max-Age=0; HttpOnly; SameSite=Strict'});
    }
    if(req.method==='GET' && route==='/'){
      if(!authenticated(req))return redirect(res,'/login');
      return publicFile(res,'index.html');
    }
    return respond(res,404,'Not found');
  }catch(e){
    if(e instanceof SyntaxError)return json(res,400,{error:'Malformed JSON'});
    if(e.status===413)return json(res,413,{error:'Request too large'});
    console.error('Request failure:',e.message);return json(res,500,{error:'Server error'});
  }
});
server.on('upgrade',(req,socket,head)=>{
  let parsed;
  try{parsed=new URL(req.url,'http://localhost');}catch{return socket.destroy();}
  if(parsed.pathname!=='/ws'||!authenticated(req)){socket.write('HTTP/1.1 401 Unauthorized\r\nConnection: close\r\n\r\n');return socket.destroy();}
  const origin=req.headers.origin;
  if(!origin){socket.write('HTTP/1.1 403 Forbidden\r\nConnection: close\r\n\r\n');return socket.destroy();}
  let originHost;
  try{originHost=new URL(origin).host;}catch{return socket.destroy();}
  if(originHost!==req.headers.host){socket.write('HTTP/1.1 403 Forbidden\r\nConnection: close\r\n\r\n');return socket.destroy();}
  wss.handleUpgrade(req,socket,head,ws=>wss.emit('connection',ws));
});
wss.on('connection',ws=>{
  clients.add(ws);ws.isAlive=true;
  ws.on('pong',()=>{ws.isAlive=true});
  ws.send(JSON.stringify({type:'status',deviceOnline:isOnline()}));
  expireCommand();
  if(pendingLimitCommand)ws.send(JSON.stringify({type:'speed_limit_command',state:'pending',id:pendingLimitCommand.id,value:pendingLimitCommand.value,message:'Waiting for ESP32 acknowledgement'}));
  if(isOnline())ws.send(JSON.stringify({type:'telemetry',data:lastTelemetry}));
  // WebSocket remains read-only; all changes use authenticated, same-origin HTTP POST.
  ws.on('message',()=>ws.send(JSON.stringify({type:'error',message:'Use the dashboard threshold controls; direct WebSocket commands are disabled.'})));
  ws.on('close',()=>clients.delete(ws));
});
const interval=setInterval(()=>{
  const online=isOnline();
  expireCommand();
  if(lastOnlineState!==online){lastOnlineState=online;broadcast({type:'status',deviceOnline:online});}
  for(const ws of clients){if(!ws.isAlive){ws.terminate();continue;}ws.isAlive=false;ws.ping();}
  // Prevent unbounded login attempt records.
  for(const [key,value] of loginAttempts)if(value.until<Date.now() && value.count===0)loginAttempts.delete(key);
},2500);
interval.unref();
server.listen(PORT,'0.0.0.0',()=>console.log(`ParkSense Coolify running on 0.0.0.0:${PORT}`));

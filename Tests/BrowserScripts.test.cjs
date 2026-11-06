const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const assert = require('node:assert/strict');
const source = name => fs.readFileSync(path.join(__dirname, '../Source', name), 'utf8');
let routes=0;
function wheel(hostname, pathname, video, expected) {
  ++routes;
  const handlers = {}, messages = [];
  const context = {location:{hostname,pathname,protocol:'https:'},document:{querySelector:()=>video?{}:null,body:{}},console:{debug:x=>messages.push(x)},performance:{now:()=>now},innerHeight:600,setInterval:()=>{},addEventListener:(n,fn)=>handlers[n]=fn};
  context.window=context;vm.runInNewContext(source('mobile-input.js'),context);
  assert.equal(messages[0], '__WC_MODE__:'+(expected||'normal'));
  let now=1000;
  let prevented=false;handlers.wheel({isTrusted:true,target:{},deltaY:120,deltaX:0,deltaMode:0,preventDefault:()=>prevented=true});
  assert.equal(prevented,!!expected);
  if(expected)assert.equal(messages.at(-1),'__WC_WHEEL__:'+expected+':1');
  for(const [key,axis,direction] of [['ArrowDown',expected,1],['ArrowUp',expected,-1],['ArrowLeft',expected==='stories'?'stories':'horizontal',-1],['ArrowRight',expected==='stories'?'stories':'horizontal',1]]){
    handlers.keydown({isTrusted:true,key,target:{},preventDefault:()=>{}});assert.equal(messages.at(-1),'__WC_WHEEL__:'+axis+':'+direction);
  }
  now+=701;handlers.wheel({isTrusted:true,target:{},deltaY:-120,deltaX:0,deltaMode:0,preventDefault:()=>{}});
  if(expected)assert.equal(messages.at(-1),'__WC_WHEEL__:'+expected+':-1');
  const count=messages.length;handlers.keydown({isTrusted:true,key:'ArrowDown',target:{closest:()=>true},preventDefault:()=>{throw Error('editable intercepted');}});assert.equal(messages.length,count);
  handlers.keydown({isTrusted:false,key:'ArrowDown',target:{},preventDefault:()=>{throw Error('synthetic key accepted');}});
  handlers.wheel({isTrusted:false,target:{},deltaY:120,deltaX:0,preventDefault:()=>{throw Error('synthetic wheel accepted');}});assert.equal(messages.length,count);
  handlers.keydown({isTrusted:true,key:'ArrowDown',target:{isContentEditable:true,closest:()=>null},preventDefault:()=>{throw Error('editable without attribute value intercepted');}});
  handlers.wheel({isTrusted:true,shiftKey:true,target:{},deltaY:120,deltaX:0,preventDefault:()=>{throw Error('Shift+wheel intercepted');}});assert.equal(messages.length,count);
}
wheel('www.tiktok.com','/@test/video/1',true,'reels');
wheel('www.instagram.com','/reels/1/',true,'reels');
wheel('www.instagram.com','/stories/test/1/',false,'stories');
wheel('m.facebook.com','/reel/1/',true,'reels');
wheel('m.youtube.com','/shorts/1',true,'reels');
wheel('example.com','/',false,'scroll');
wheel('not-tiktok.com','/',true,'scroll');
wheel('m.soundcloud.com','/',false,'scroll');
wheel('open.spotify.com','/',false,'scroll');
wheel('www.instagram.com','/reel/1/',true,'reels');
wheel('www.instagram.com','/reels',true,'reels');
wheel('www.instagram.com','/stories',false,'stories');
wheel('www.instagram.com','/reels/audio/1/',true,'scroll');
wheel('www.instagram.com','/p/1/',false,'scroll');
wheel('www.facebook.com','/reels/',true,'reels');
wheel('www.facebook.com','/reel/1/',true,'reels');
wheel('m.facebook.com','/stories/1/',false,'stories');
wheel('www.facebook.com','/watch/',true,'scroll');
wheel('www.facebook.com','/reels/create/',false,'scroll');
wheel('www.facebook.com.evil.test','/reel/1/',true,'scroll');
wheel('x.com','/',true,'scroll');
wheel('www.reddit.com','/',true,'scroll');
wheel('m.twitch.tv','/',true,'scroll');
wheel('mail.google.com','/',false,'scroll');

function credentials(protocol='https:', existing='') {
  const handlers={},calls=[];
  class Input {constructor(value=''){this._value=value;this.dataset={};this.form=null;}get value(){return this._value;}set value(x){this._value=x;}dispatchEvent(){}}
  const user=new Input(existing),password=new Input(existing);
  const document={documentElement:{},querySelector:s=>s.includes('type="password"')?password:user};
  const context={location:{protocol},document,HTMLInputElement:Input,Event:class{},MutationObserver:class{observe(){}},addEventListener:(n,fn)=>handlers[n]=fn,
    prompt:(message,data)=>{calls.push(message);if(message==='__WC_PASSWORD_LOAD__')return JSON.stringify({user:'synthetic-user',password:'synthetic-value'});assert.deepEqual(JSON.parse(data),{user:user.value,password:password.value});return 'saved';}};
  context.window=context;context.top=context;
  vm.runInNewContext(source('passwords.js'),context);
  return {calls,handlers,user,password};
}
let c=credentials();assert.equal(c.user.value,'synthetic-user');assert.equal(c.password.value,'synthetic-value');c.handlers.submit();assert.equal(c.calls.at(-1),'__WC_PASSWORD_SAVE__');
c=credentials('https:','already-typed');assert.equal(c.user.value,'already-typed');assert.equal(c.password.value,'already-typed');
assert.equal(credentials('http:').calls.length,0);
console.log('PASS: '+routes+' routes, '+(routes*2)+' wheel directions, '+(routes*4)+' arrow cases, editable exclusions; credential fill/save and HTTPS scope');

const fs=require('node:fs'),vm=require('node:vm'),assert=require('node:assert/strict'),path=require('node:path');
const source=f=>fs.readFileSync(path.join(__dirname,'../Source',f),'utf8');
// Feed DOM churn must not schedule root zoom/resize work.
let observers=0,zoomWrites=0;const appended=[];const root={style:new Proxy({}, {set(){++zoomWrites;return true;}})};
const fit={document:{documentElement:root,querySelector:()=>null,head:{appendChild:x=>appended.push(x)},createElement:()=>({})},MutationObserver:class{constructor(){++observers;}observe(){}},setTimeout:()=>{throw Error('unbounded fit work');}};fit.window=fit;
vm.runInNewContext(source('mobile-fit.js'),fit);for(let i=0;i<500;++i)vm.runInNewContext(source('mobile-fit.js'),fit);
assert.equal(observers,0);assert.equal(zoomWrites,0);assert.equal(appended.length,2);
let now=1000;const events={},messages=[];const body={},comment={scrollHeight:800,clientHeight:200,parentElement:body,querySelector:()=>null};
const context={location:{protocol:'https:',hostname:'www.tiktok.com',pathname:'/'},document:{body,querySelector:()=>({})},getComputedStyle:()=>({overflowY:'auto'}),console:{debug:x=>messages.push(x)},performance:{now:()=>now},innerHeight:600,setInterval:()=>{},addEventListener:(k,f)=>events[k]=f};context.window=context;
vm.runInNewContext(source('mobile-input.js'),context);
const wheel=target=>events.wheel({target,isTrusted:true,deltaY:120,deltaX:0,deltaMode:0,preventDefault(){this.prevented=true;}});
const before=messages.length;wheel(comment);assert.equal(messages.length,before,'comment list intercepted');
for(let i=0;i<60;++i){now+=701;wheel({});}
assert.equal(messages.filter(x=>x==='__WC_WHEEL__:reels:1').length,60);
const total=messages.length;for(let i=0;i<100;++i){now+=1;wheel({});}assert.equal(messages.length,total,'wheel burst generated overlapping gestures');
events.keydown({target:comment,isTrusted:true,key:'ArrowDown',preventDefault(){throw Error('comment keyboard intercepted');}});
events.error({target:{tagName:'VIDEO',error:{code:3},getBoundingClientRect:()=>({width:200,height:300,top:0,bottom:300})}});assert.equal(messages.at(-1),'__WC_MEDIA__:3');events.playing({target:{tagName:'VIDEO'}});assert.equal(messages.at(-1),'__WC_MEDIA__:ok');
console.log('PASS: 500 fit injections, nested comments, 60 reel steps, 100-event wheel burst');

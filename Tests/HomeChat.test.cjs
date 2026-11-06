const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
(async()=>{for(const [unread,expected,hidden] of [[0,'0',true],[1,'1',false],[99,'99',false],[100,'99',false],[1000,'99',false]]){
 const badge={hidden:true,textContent:''},link={setAttribute(){}},handlers={};let requested='';
 const ctx={document:{hidden:false,getElementById:id=>id==='ptg-badge'?badge:link},addEventListener:(name,fn)=>handlers[name]=fn,setTimeout:()=>1,clearTimeout(){},setInterval(){},AbortController,fetch:async url=>{requested=url;return{ok:true,json:async()=>({unread})}}};
 vm.runInNewContext(fs.readFileSync(path.join(__dirname,'../Source/home-chat.js'),'utf8'),ctx);handlers['ptg-state']({detail:{read:123}});await new Promise(setImmediate);
 assert.equal(badge.textContent,expected);assert.equal(badge.hidden,hidden);assert(requested.endsWith('after=123'));
}console.log('PASS: unread badge 0, 1, 99, 100, 1000; capped99 and persisted cursor');})().catch(e=>{console.error(e);process.exitCode=1});

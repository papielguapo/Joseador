(()=>{
 if(window.__wcWheelInstalled)return;
 window.__wcWheelInstalled=true;
 let accumulated=0,last=-1000,reported="",hover=null;
 const mode=()=>{
   if(!/^https?:$/.test(location.protocol))return '';
   const p=location.pathname.toLowerCase(),h=location.hostname.toLowerCase();
   if(/(^|\.)tiktok\.com$/.test(h)&&document.querySelector('video'))return 'reels';
   if(/(^|\.)(instagram|facebook)\.com$/.test(h)){
     if(/\/stories\//.test(p))return 'stories';
     if(/\/reels?(\/|$)/.test(p))return 'reels';
   }
   if(/(^|\.)youtube\.com$/.test(h)&&p.startsWith('/shorts/'))return 'reels';
   return 'scroll';
 };
 const blocked=t=>{
   if(t?.closest?.('input,textarea,select,[contenteditable="true"],[role="textbox"],[role="slider"],[role="menu"]'))return true;
   for(let n=t;n&&n!==document.body;n=n.parentElement){if(n.matches?.('[role="dialog"],textarea')&&!n.querySelector?.('video')&&n.scrollHeight>n.clientHeight+4)return true;}return false;
 };
 const report=()=>{const value=blocked(hover)?'normal':(mode()||'normal');if(value!==reported){reported=value;console.debug('__WC_MODE__:'+value);}};
 addEventListener('pointermove',e=>{hover=e.target;report();},{passive:true,capture:true});
 addEventListener('popstate',report);addEventListener('hashchange',report);setInterval(report,500);report();
 addEventListener('keydown',e=>{
   if(e.ctrlKey||e.altKey||e.metaKey||e.shiftKey||blocked(e.target))return;
   const direction={ArrowDown:1,ArrowRight:1,ArrowUp:-1,ArrowLeft:-1}[e.key];
   const m=mode();if(!direction||!m)return;e.preventDefault();
   const horizontal=e.key==='ArrowLeft'||e.key==='ArrowRight';
   console.debug('__WC_WHEEL__:'+(m==='stories'?'stories':horizontal?'horizontal':m)+':'+direction);
 },{capture:true});
 addEventListener('wheel',e=>{
   if(e.ctrlKey||e.metaKey||Math.abs(e.deltaX)>Math.abs(e.deltaY)||!e.deltaY)return;
   if(blocked(e.target))return;
   const m=mode();if(!m)return;
   e.preventDefault();
   const now=performance.now();if(now-last<450)return;
   if(accumulated&&Math.sign(accumulated)!==Math.sign(e.deltaY))accumulated=0;
   accumulated+=e.deltaY*(e.deltaMode===1?16:e.deltaMode===2?innerHeight:1);
   if(Math.abs(accumulated)<35)return;
   const direction=accumulated>0?1:-1;accumulated=0;last=now;
   // The browser process converts this user wheel action to a trusted touch gesture.
   console.debug('__WC_WHEEL__:'+m+':'+direction);
 },{capture:true,passive:false});
})();


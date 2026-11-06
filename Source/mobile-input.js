(()=>{
 if(window.__wcWheelInstalled)return;
 window.__wcWheelInstalled=true;
 let accumulated=0,last=-1000,reported="",hover=null,pointerX=-1,pointerY=-1;
 // Keep only numeric diagnostics for a visible failed video; never its URL.
 addEventListener('error',e=>{const v=e.target;if(v?.tagName!=='VIDEO'||!v.error)return;const r=v.getBoundingClientRect();if(r.width>0&&r.height>0&&r.bottom>0&&r.top<innerHeight)console.debug('__WC_MEDIA__:'+v.error.code);},{capture:true});
 addEventListener('playing',e=>{if(e.target?.tagName==='VIDEO')console.debug('__WC_MEDIA__:ok');},{capture:true});
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
   if(t?.isContentEditable)return true;
   if(t?.closest?.('input,textarea,select,[contenteditable="true"],[role="textbox"],[role="slider"],[role="menu"]'))return true;
   for(let n=t;n&&n!==document.body;n=n.parentElement){
     // Nested comment lists and menus must keep their own wheel scrolling.
     if(n.scrollHeight>n.clientHeight+4&&!n.querySelector?.('video')){
       const overflow=getComputedStyle(n).overflowY;
       if(overflow==='auto'||overflow==='scroll'||n.matches?.('[role="dialog"],textarea'))return true;
     }
   }return false;
 };
 const report=()=>{const target=pointerX>=0&&pointerY>=0?document.elementFromPoint?.(pointerX,pointerY)||hover:hover;const value=blocked(target)?'normal':(mode()||'normal');if(value!==reported){reported=value;console.debug('__WC_MODE__:'+value);}};
 const point=e=>{hover=e.target;pointerX=e.clientX??-1;pointerY=e.clientY??-1;report();};
 addEventListener('pointermove',point,{passive:true,capture:true});addEventListener('pointerdown',point,{passive:true,capture:true});
 addEventListener('popstate',report);addEventListener('hashchange',report);setInterval(report,500);report();
 addEventListener('keydown',e=>{
   if(!e.isTrusted||e.ctrlKey||e.altKey||e.metaKey||e.shiftKey||blocked(e.target))return;
   const direction={ArrowDown:1,ArrowRight:1,ArrowUp:-1,ArrowLeft:-1}[e.key];
   const m=mode();if(!direction||!m)return;e.preventDefault();
   const horizontal=e.key==='ArrowLeft'||e.key==='ArrowRight';
   console.debug('__WC_WHEEL__:'+(m==='stories'?'stories':horizontal?'horizontal':m)+':'+direction);
 },{capture:true});
 addEventListener('wheel',e=>{
   if(!e.isTrusted||e.ctrlKey||e.altKey||e.shiftKey||e.metaKey||Math.abs(e.deltaX)>Math.abs(e.deltaY)||!e.deltaY)return;
   if(blocked(e.target))return;
   const m=mode();if(!m)return;
   e.preventDefault();
   const now=performance.now();if(now-last<(m==='scroll'?450:700))return;
   if(accumulated&&Math.sign(accumulated)!==Math.sign(e.deltaY))accumulated=0;
   accumulated+=e.deltaY*(e.deltaMode===1?16:e.deltaMode===2?innerHeight:1);
   if(Math.abs(accumulated)<35)return;
   const direction=accumulated>0?1:-1;accumulated=0;last=now;
   // The browser process converts this user wheel action to a trusted touch gesture.
   console.debug('__WC_WHEEL__:'+m+':'+direction);
 },{capture:true,passive:false});
})();


(()=>{
 const badge=document.getElementById('ptg-badge'),link=document.getElementById('ptg-link');if(!badge||!link)return;
 const origin='https://part-time-gang.tangycondor.chatgpt.site';let read=0,busy=false;
 const update=async()=>{if(busy||document.hidden)return;busy=true;const controller=new AbortController();const timer=setTimeout(()=>controller.abort(),8000);
  try{const response=await fetch(origin+'/api/status?after='+read,{credentials:'omit',cache:'no-store',signal:controller.signal});if(!response.ok)return;const data=await response.json();
   const n=Number(data.unread);if(!Number.isFinite(n)||n<0)return;const count=Math.min(99,Math.floor(n));badge.textContent=String(count);badge.hidden=count===0;link.setAttribute('aria-label','Part Time Gang'+(count?' — '+count+(count===1?' mensaje sin leer':' mensajes sin leer'):''));
  }catch{}finally{clearTimeout(timer);busy=false;}
 };
 addEventListener('ptg-state',event=>{const n=Number(event.detail?.read);if(Number.isSafeInteger(n)&&n>=0){read=n;update();}});
 addEventListener('visibilitychange',update);setInterval(update,12000);setTimeout(update,600);
})();

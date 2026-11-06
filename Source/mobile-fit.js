(()=>{
 if(window.__wcFitInstalled)return;
 window.__wcFitInstalled=true;
 // A feed changes its DOM for every video. Never toggle root zoom or observe
 // the whole feed: that changes touch coordinates and can trigger resize loops.
 let meta=document.querySelector('meta[name=viewport]');
 if(!meta){meta=document.createElement('meta');meta.name='viewport';document.head.appendChild(meta);}
 meta.content='width=device-width,initial-scale=1';
 const style=document.createElement('style');style.id='webcapture-fit';
 style.textContent='html{max-width:100%!important;overflow-x:hidden!important}body{width:100%!important;max-width:100%!important;min-width:0!important;box-sizing:border-box!important}video,iframe,img,canvas{max-width:100%!important}';
 document.head.appendChild(style);
})();

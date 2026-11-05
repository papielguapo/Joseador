(()=>{
 if(window.__wcPasswords||location.protocol!=='https:'||window.top!==window)return;
 window.__wcPasswords=true;
 let asked=false,loading=false;
 const username=root=>root.querySelector('input[autocomplete="username"],input[type="email"],input[name*="email" i],input[name*="user" i],input[type="text"]');
 const password=()=>document.querySelector('input[type="password"]:not([autocomplete="new-password"])');
 const fill=(field,value)=>{if(!field||field.value||!value)return;Object.getOwnPropertyDescriptor(HTMLInputElement.prototype,'value').set.call(field,value);field.dispatchEvent(new Event('input',{bubbles:true}));field.dispatchEvent(new Event('change',{bubbles:true}));};
 const restore=()=>{
   const field=password();if(!field||field.dataset.wcChecked||loading)return;field.dataset.wcChecked='1';loading=true;
   try{const data=JSON.parse(prompt('__WC_PASSWORD_LOAD__','')||'{}');fill(username(field.form||document),data.user);fill(field,data.password);}catch(_){}finally{loading=false;}
 };
 const save=()=>{
   if(asked)return;const field=password();if(!field||!field.value)return;asked=true;
   const user=username(field.form||document)?.value||'';
   // This private prompt is handled natively. Credentials never enter console logs, URLs, or localStorage.
   const result=prompt('__WC_PASSWORD_SAVE__',JSON.stringify({user,password:field.value}));
   if(result==='retry')asked=false;
 };
 addEventListener('submit',save,{capture:true});
 addEventListener('click',e=>{const button=e.target.closest?.('button,input[type="submit"]');if(button&&(button.type==='submit'||/log.?in|sign.?in|iniciar|entrar|siguiente|next|continuar/i.test(button.textContent||button.value||'')))save();},{capture:true});
 addEventListener('focusin',restore);new MutationObserver(restore).observe(document.documentElement,{childList:true,subtree:true});restore();
})();

const headers={'Content-Type':'application/json; charset=utf-8','Cache-Control':'no-store','Access-Control-Allow-Origin':'*','Access-Control-Allow-Methods':'GET, POST, OPTIONS','Access-Control-Allow-Headers':'Authorization, Content-Type','X-Content-Type-Options':'nosniff'};
const json=(data,status=200)=>new Response(JSON.stringify(data),{status,headers});
const uuid=v=>typeof v==='string'&&/^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/i.test(v);
export const digest=async s=>Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',new TextEncoder().encode(s))),b=>b.toString(16).padStart(2,'0')).join('');
async function readBody(request){
 if(Number(request.headers.get('content-length'))>6000)throw Error('size');
 const reader=request.body?.getReader();if(!reader)throw new SyntaxError('body');
 const decoder=new TextDecoder('utf-8',{fatal:true});let length=0,raw='';
 try{while(true){const {value,done}=await reader.read();if(done)break;length+=value.byteLength;if(length>6000){await reader.cancel();throw Error('size')}raw+=decoder.decode(value,{stream:true});}raw+=decoder.decode();}finally{reader.releaseLock()}
 const body=JSON.parse(raw);if(!body||typeof body!=='object'||Array.isArray(body))throw new SyntaxError('object');return body;
}
async function rate(db,key,seconds,maximum,now){const row=await db.prepare('INSERT INTO rate_limits (key,count,window_start) VALUES (?,1,?) ON CONFLICT(key) DO UPDATE SET count=CASE WHEN window_start<=? THEN 1 ELSE count+1 END, window_start=CASE WHEN window_start<=? THEN ? ELSE window_start END RETURNING count').bind(key,now,now-seconds*1000,now-seconds*1000,now).first();return Number(row.count)<=maximum;}
async function guest(request,db){const token=request.headers.get('authorization')?.replace(/^Bearer /,'');if(!token||!/^[0-9a-f-]{72}$/i.test(token))return null;return db.prepare('SELECT token_hash,author_id,nick FROM guests WHERE token_hash=?').bind(await digest(token)).first();}
const message=row=>({id:Number(row.id),key:row.message_key,authorId:row.author_id,name:row.nick,text:row.body,createdAt:Number(row.created_at)});
export async function handleChat(request,db){
 if(request.method==='OPTIONS')return new Response(null,{status:204,headers});
 if(!db)return json({error:'El chat no esta disponible temporalmente.'},503);
 const url=new URL(request.url),path=url.pathname,now=Date.now();
 const ip=request.headers.get('cf-connecting-ip')||'local';
 try{
  if(request.method==='GET'&&(path==='/api/status'||path==='/api/messages')){
   const raw=url.searchParams.get('after');if(raw!==null&&!/^\d{1,16}$/.test(raw))return json({error:'Cursor invalido.'},400);
   const after=raw===null?null:Number(raw);if(after!==null&&!Number.isSafeInteger(after))return json({error:'Cursor invalido.'},400);
   const latest=await db.prepare('SELECT COALESCE(MAX(id),0) AS id FROM messages').first();
   if(path==='/api/status'){const count=await db.prepare('SELECT COUNT(*) AS n FROM (SELECT id FROM messages WHERE id>? LIMIT 99)').bind(after??0).first();return json({unread:Math.min(99,Number(count.n)),latestId:Number(latest.id)});}
   const data=after===null?await db.prepare('SELECT * FROM (SELECT * FROM messages ORDER BY id DESC LIMIT 60) ORDER BY id').all():await db.prepare('SELECT * FROM messages WHERE id>? ORDER BY id LIMIT 100').bind(after).all();
   const rows=data.results||[];return json({messages:rows.map(message),latestId:Number(latest.id),more:after!==null&&rows.length===100&&Number(rows.at(-1).id)<Number(latest.id)});
  }
  if(request.method==='POST'&&path==='/api/session'){
   const body=await readBody(request),name=typeof body.name==='string'?body.name.trim().replace(/\s+/g,' '):'';
   if(!name||name.length>24||/[\x00-\x1f\x7f<>]/.test(name))return json({error:'Escribe un apodo de 1 a 24 caracteres.'},400);
   if(request.headers.has('authorization')){const who=await guest(request,db);if(!who)return json({error:'Vuelve a entrar con tu apodo.'},401);await db.prepare('UPDATE guests SET nick=? WHERE token_hash=?').bind(name,who.token_hash).run();return json({authorId:who.author_id,name});}
   const ipKey=await digest('registration:'+ip);if(!await rate(db,ipKey,3600,60,now))return json({error:'Demasiados accesos. Intentalo mas tarde.'},429);
   const token=crypto.randomUUID()+crypto.randomUUID(),authorId=crypto.randomUUID();
   await db.prepare('INSERT INTO guests (token_hash,author_id,nick,created_at,last_post) VALUES (?,?,?,?,0)').bind(await digest(token),authorId,name,now).run();
   await db.prepare('DELETE FROM rate_limits WHERE window_start<?').bind(now-86400000).run();
   return json({token,authorId,name},201);
  }
  if(request.method==='POST'&&path==='/api/messages'){
   const who=await guest(request,db);if(!who)return json({error:'Elige un apodo para entrar.'},401);
   const body=await readBody(request),text=typeof body.text==='string'?body.text.trim():'';
   if(!uuid(body.key)||!text||text.length>1000||/[\x00-\x08\x0b\x0c\x0e-\x1f\x7f]/.test(text))return json({error:'Escribe un mensaje de 1 a 1000 caracteres.'},400);
   const previous=await db.prepare('SELECT * FROM messages WHERE message_key=?').bind(body.key).first();
   if(previous)return previous.author_id===who.author_id?json({message:message(previous)}):json({error:'Identificador de envio en uso.'},409);
   if(!await rate(db,await digest('messages:'+ip),60,120,now))return json({error:'Demasiados mensajes. Espera un momento.'},429);
   const allowed=await db.prepare('UPDATE guests SET last_post=? WHERE token_hash=? AND last_post<=? RETURNING author_id').bind(now,who.token_hash,now-2000).first();
   if(!allowed)return json({error:'Espera dos segundos entre mensajes.'},429);
   const row=await db.prepare('INSERT INTO messages (message_key,author_id,nick,body,created_at) VALUES (?,?,?,?,?) RETURNING *').bind(body.key,who.author_id,who.nick,text,now).first();
   await db.prepare('DELETE FROM messages WHERE id<=(SELECT COALESCE(MAX(id),0)-5000 FROM messages)').run();
   return json({message:message(row)},201);
  }
  return json({error:'Ruta no disponible.'},404);
 }catch(error){if(error instanceof SyntaxError||error?.message==='size')return json({error:'Solicitud invalida o demasiado grande.'},400);return json({error:'No se pudo completar la solicitud. Vuelve a intentarlo.'},503);}
}

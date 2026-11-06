import { env } from 'cloudflare:workers';
import { handleChat } from '../../../lib/chat-server.mjs';
export const dynamic='force-dynamic';
export const GET=(request:Request)=>handleChat(request,env.DB);
export const POST=GET;
export const OPTIONS=GET;

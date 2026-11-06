# Part Time Gang

Servidor y web del chat de JOSEADOR, por PAPIELGUAPO.

Sala publicada: https://part-time-gang.tangycondor.chatgpt.site

Usa Vinext/React, Cloudflare Workers y D1 mediante Sites. La fuente desplegada corresponde al commit 70f234533f506271ef96a059dcd4a882ede41b9e del repositorio de hosting. Esta copia permite revisar el servidor sin incluir credenciales, datos de usuarios ni la base de prueba.

## Funciones

Apodos de invitado, cambio de apodo para mensajes posteriores, historial público, envío autenticado mediante token de invitado, reintentos sin duplicación, paginación y contador 0–99. El servidor guarda hashes de tokens. Los apodos no verifican identidad. Se conservan hasta 5.000 mensajes: cada mensaje nuevo elimina automáticamente los más antiguos que excedan ese límite. La sala sigue aceptando mensajes; no hay que vaciarla ni reiniciarla.

## Desarrollo

Requiere Node 22.13 o posterior. Instala dependencias con npm ci. Ejecuta node scripts/test-chat.mjs para las 36 comprobaciones en una base SQLite en memoria, incluida la eliminación de los mensajes más antiguos y la recepción continuada al superar los 5.000. Para publicar usa el flujo de Sites y sus helpers; .openai/hosting.json identifica la sala actual y requiere autorización del propietario para desplegar. No insertes tokens o claves en ese archivo.

La migración inicial está en drizzle/0000_clumsy_mandarin.sql. Las bases locales y de producción están separadas. No cambies migraciones ya aplicadas; añade nuevas.

Endpoints: GET /api/messages, GET /api/status, POST /api/session y POST /api/messages. Las escrituras de mensajes requieren un token de invitado; los datos de lectura son públicos.

Consulta la guía y la auditoría en el directorio docs del proyecto principal.

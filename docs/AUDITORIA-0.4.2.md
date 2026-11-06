# Auditoría y correcciones de JOSEADOR 0.4.2

Revisión de navegador CEF, controles de entrada, ciclo de vida del puente de audio, navegación, instalación, credenciales y chat. Motivada por el fallo de TikTok reportado alrededor del tercer vídeo y por la franja plana entre Inicio y el buscador.

## Fallos encontrados y corregidos

1. **Reajuste continuo del zoom en feeds.** Se observaban todas las mutaciones del documento; cada cambio de vídeo podía restablecer zoom, medir el contenido y volver a escalarlo. Se elimina ese observador y la oscilación del zoom. La vista móvil usa el viewport y CSS estables, conservando el ajuste al tamaño de la ventana.
2. **Rueda interceptada antes de comprobar el elemento.** El hook nativo convertía la rueda en gesto y consumía el evento antes de que el script comprobara comentarios, menús o campos. Ahora solo acredita la entrada física: la página decide si conserva su scroll normal o solicita un gesto. Se respetan listas desplazables anidadas sin vídeo.
3. **Gestos durante cambios de página.** El gesto activo y las autorizaciones de entrada podían continuar al navegar, recargar o redimensionar. Se cancelan, se rechazan dimensiones nulas y se evitan actualizaciones redundantes de métricas. Los cambios de reel se espacian al menos 700 ms.
4. **Carrera entre audio y reinicio.** Publicar un puntero nulo no garantizaba que un callback de audio ya iniciado hubiera dejado de utilizar el mapa compartido. Un guardián de lectores protege ahora su vida útil; el callback de audio no espera ni adquiere mutex. El propietario espera a los lectores antes de liberar el mapa.
5. **Fallos del navegador sin recuperación ni detalle.** Se registran códigos de carga, error de vídeo (cancelación/red/decodificación/formato) y terminación en el estado local, sin guardar URLs ni contraseñas. Si termina el renderer se intenta recargar una vez, con límite de un intento cada 30 segundos. La navegación manual cancela recuperaciones pendientes. No se reintentan automáticamente errores de red, certificados ni errores de aplicación propios de una web.
6. **Error de audio persistente y cierre incompleto.** Un stream nuevo limpia su error anterior; al detenerse se descarta audio pendiente y se reinicia el interpolador. Si el proceso propio no termina durante el cierre, se finaliza antes de liberar sus recursos.
7. **Franja monocromática al pie de Inicio.** La raíz usa el mismo degradado y el cuerpo establece su propio contexto de formato para evitar el colapso del margen de la cuadrícula. El fondo llega al borde inferior de la pantalla; se conserva la separación oscura del buscador de la referencia.

También se sustituye «OFF» por el símbolo de Wi-Fi con una X cuando el PC está sin conexión. Se mantienen Wi-Fi conectado, LAN y estado desconocido; no se modifica la configuración de red.

## Validación

- Siete suites CTest aprobadas. Se mantienen navegación, entrada física, credenciales, cola, estado de chat e instalación; se añade protección de audio con 20.000 ciclos concurrentes y limitación de recuperación.
- Scripts: nueve rutas web, 36 combinaciones de flechas, exclusiones de edición y modificadores; 500 inyecciones de ajuste sin observadores ni cambios de zoom; comentarios desplazables, 60 pasos de reel y ráfaga de 100 eventos de rueda.
- CEF real: 60 gestos táctiles consecutivos, cambios de DOM entre ellos, todos confiables y sin solapamiento. Prueba controlada local, no 60 vídeos reales de TikTok.
- CEF real: caída provocada del renderer y recarga recuperada. Se verifican también los gestos en ambos sentidos, Atrás hasta Inicio, recarga y once accesos ajustados a 256 px.
- Host VST3: instrumento, MIDI y estado a 44,1/48/96 kHz. Puente con señal de prueba; salida directa del árbol del navegador a Windows con RMS cero.
- Chat: 26 comprobaciones locales en SQLite en memoria y contador 0/1/99/100/1000. No se publican mensajes de prueba en la sala real.
- Instalador y VST3 compilados como 0.4.2; paquete completo para Common Files x64, sin necesitar una versión anterior. Se requiere mobile-fit.js para evitar una instalación parcial de esta corrección.

## Alcance y lo que falta por confirmar

Se inspeccionó TikTok dentro de FL Studio y se observó reproducción y cambio de vídeo. Con la versión nueva se observó además «Unable to play media» en un elemento del feed; la vista accesible no permite atribuirlo con seguridad al vídeo activo ni conocer su causa. Se añade diagnóstico numérico de errores de vídeos visibles, sin guardar sus URLs. **No se ha confirmado la causa exacta del fallo del tercer vídeo ni su desaparición en una sesión larga del servicio real con esta versión.** Las correcciones anteriores resuelven defectos del plugin y las regresiones descritas; la prueba local no sustituye esa comprobación real.

Los errores JavaScript, restricciones de cuenta, desafíos, fallos de red o bloqueos del servicio pueden requerir actuaciones distintas. Esta auditoría no es una prueba de penetración, no actualiza Chromium, no certifica todos los DAWs ni prueba un Windows limpio. Los apodos públicos del chat siguen sin ser identidades verificadas y no se añade moderación administrativa.

[Guía](GUIA.md) · [Instalación compartida](INSTALACION-0.4.1.md) · [Auditoría anterior](AUDITORIA-0.4.0.md)

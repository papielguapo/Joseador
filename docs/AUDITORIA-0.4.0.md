# Auditoría y correcciones — JOSEADOR 0.4.0

Revisión interna del código propio y regresiones en Windows 11 x64. Continúa la revisión de 0.3.1; no certifica todos los DAWs ni las dependencias de terceros.

## Bugs corregidos

| Fallo | Corrección y evidencia |
| --- | --- |
| Una acción rápida podía sobrescribir otra antes de que el navegador la recibiera; la URL solicitada compartía campo con la URL mostrada. | Cola FIFO de 16 peticiones, separada de la dirección publicada por el navegador. Pruebas de orden, vueltas de la cola, saturación y navegación seguida de recarga. Una cola llena rechaza peticiones nuevas sin sobrescribir las pendientes. |
| Cargar un estado mientras el editor estaba abierto cambiaba la variable guardada, pero no la página. | Se envía la navegación al aplicar el estado. En el VST3 anterior se reprodujo: primera página correcta y cambio rechazado. Con 0.4.0 ambas páginas se aplican. La prueba utiliza el contenedor real del estado VST3 del host JUCE. |
| Después de fallar por un runtime inexistente, un inicio correcto seguía mostrando el error anterior. | El inicio limpia ese error. Regresión reproducida antes (error no limpiado) y superada después. Las acciones de actualizar, buscar e Inicio pueden reiniciar un helper cerrado. |
| Las flechas interceptaban algunos editores HTML con contenteditable vacío o heredado. | Se comprueba isContentEditable. Se reprodujo la intercepción antes de corregirla y se añaden casos de regresión. |
| Mayús+rueda podía convertirse en gesto a través del listener JavaScript aunque el hook nativo la dejaba pasar. | Los modificadores se respetan en JavaScript y en el hook nativo, incluido Alt. |

La recarga espera de forma acotada si una navegación está cargando: no queda bloqueada indefinidamente por una página que mantenga su indicador de carga. El puente tiene una versión explícita para detectar mezclas de binarios. Actualiza el VST3 y BrowserRuntime juntos y reinicia el DAW.

## Part Time Gang

- Sala pública alojada con servidor y base D1 compartida. No es una simulación local ni depende de una pestaña del creador.
- Apodos de 1 a 24 caracteres, visibles también en los mensajes propios y modificables para envíos posteriores. No son identidades verificadas.
- Token de invitado aleatorio; el servidor almacena su hash. No se incluyen contraseñas ni credenciales del servicio en el plugin.
- Autor del mensaje obtenido de la sesión; no se acepta el nombre enviado por otro cliente como identidad.
- Consultas SQL parametrizadas, tamaño de petición acotado durante la lectura, límites de mensajes, reintentos con identificador para evitar duplicados y paginación por ID.
- Límite de envío por invitado y por origen de red, y almacenamiento de hasta 5.000 mensajes. No sustituye a un sistema completo de moderación.
- Contador oculto a cero y limitado a 99. Lectura conservada por perfil, aceptada por el helper solo desde el origen del chat y con el editor visible.
- Comprobación visual real: Inicio mostró un mensaje sin leer; se abrió la sala desde PT y al volver se ocultó el contador. No se publicaron mensajes automáticos de prueba en producción; los ensayos de envío se hicieron en datos locales.

## Instalador EXE

Se incluye Instalar JOSEADOR.exe, junto al paquete completo y al redistribuible oficial de Microsoft. El instalador propio está enlazado estáticamente y solo depende de DLLs del sistema: puede arrancar antes de que Visual C++ esté instalado.

Comprueba archivos esenciales, copia a una carpeta temporal, conserva la versión anterior y restaura el respaldo si falla la sustitución. Una prueba inyecta un fallo después del respaldo y comprueba su restauración. También se prueban instalación nueva, actualización y paquete incompleto.

El redistribuible x64 incluido es 14.51.36247.0. Se verificaron su firma de Microsoft y SHA-256; véase ThirdParty/Microsoft-VCRUNTIME.txt. El instalador vuelve a validar firma y editor antes de ejecutarlo, y lo abre solo si falta la versión requerida. El usuario completa la interfaz y las condiciones de Microsoft. La compilación no instala ese componente en Windows.

El instalador propio no tiene firma digital. No se ha ensayado una máquina Windows limpia ni se ha automatizado la aceptación de licencias o permisos del redistribuible.

## Resultados

- **6/6 suites CTest:** política de URL, entrada nativa, credenciales sintéticas, cola de navegación, estado de lectura e instalación con recuperación.
- **26 comprobaciones del servidor:** sesión, apodos, cambio de nombre, autorización, mensajes duplicados, limitación, cursores, paginación y contador máximo 99.
- JavaScript de navegación: nueve rutas y 36 casos de flechas, editores, eventos sintéticos y modificadores.
- Contador de Inicio con 0, 1, 99, 100 y 1000 mensajes: valores esperados y ocultación correctos.
- Regresiones con helper real: error de inicio limpiado y navegación seguida de recarga conservada.
- Estado del VST3 con navegador abierto: primera y segunda página correctas después de la corrección.
- CEF real: recarga, historial hasta Inicio, once accesos y ajuste a 256 px, gestos confiables en ambos sentidos.
- Instrumento/MIDI/estado a 44,1/48/96 kHz. Señal Web Audio de 440 Hz al puente, RMS aproximado 0,125; WASAPI del navegador con RMS cero en paralelo.
- EXE final: validación del paquete y de la firma de Microsoft con código de salida 0. Dependencias del instalador inspeccionadas; sin requisito previo de DLLs de Visual C++ para arrancarlo.

## Límites

Se mantienen las limitaciones de servicios web, latencia y asignación del primer perfil libre. Esta revisión no actualiza Chromium, no añade moderación administrativa al chat y no incluye fuzzing ni prueba de penetración independiente. La comprobación visual corresponde al editor compartido de la aplicación de escritorio; el ensayo VST3 usa un host de prueba JUCE, no una certificación nueva de todos los DAWs.

[Guía](GUIA.md) · [Revisión anterior](AUDITORIA.md)

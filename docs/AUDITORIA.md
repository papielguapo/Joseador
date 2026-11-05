# Auditoría interna — Joseador 0.3.1

Revisión del código propio y pruebas de regresión en Windows 11 x64. Es una revisión interna asistida, no una certificación de seguridad ni una auditoría independiente de Chromium, JUCE o los servicios web.

## Cambio solicitado

El botón inferior de borrar se sustituye por **Actualizar página**, con icono de recarga y nombre accesible. Recarga la página que está abierta, incluso Inicio, mediante el navegador; no abre una nueva instancia ni necesita volver a escribir la dirección. Se conserva Atrás hasta Inicio.

## Hallazgos y correcciones

| Prioridad | Hallazgo | Corrección en 0.3.1 |
| --- | --- | --- |
| Alta | El puente de gestos aceptaba mensajes de consola sin comprobar una entrada nativa reciente. Una página podía intentar solicitar gestos confiables por sí sola. | Se exige una entrada de rueda o flecha registrada por los hooks nativos, con dirección/eje compatibles, límite de 500 ms y consumo único. JavaScript rechaza eventos no confiables. |
| Alta | Las peticiones privadas de contraseñas no comprobaban expresamente su coincidencia con el origen de la página principal. | Lectura y guardado limitados al mismo origen HTTPS de la página principal. Se rechazan orígenes distintos, datos de formulario excesivos y JSON de tipo incorrecto. |
| Media | Las URLs restauradas desde el estado del proyecto o solicitadas mediante ventanas emergentes no tenían una política explícita de esquemas. | Solo se aceptan HTTP/HTTPS y el acceso interno exacto a Inicio en esas entradas. Se rechazan esquemas ejecutables, archivos arbitrarios, credenciales en la autoridad, controles y entradas excesivas. |
| Media | El estado del proyecto podía conservar parámetros comunes de autenticación de una URL. | Se filtran nombres conocidos, incluidos code y tokens, también con nombres escapados, y fragmentos OAuth. Se mantienen parámetros de reproducción como el identificador de YouTube. |
| Media | CEF podía generar registros normales de errores durante la navegación. | Se desactiva su registro normal. No se han borrado archivos de registro antiguos del usuario. |
| Baja | Faltaban validaciones de tamaño/formato al leer credenciales y una comprobación del mutex de navegación al iniciar. | Límites de usuario y contraseña, validación del blob y fallo controlado si no puede crearse el mutex. |
| Baja | La actualización podía empezar a sustituir archivos antes de completar su copia. | Se copia el paquete a una carpeta temporal validada antes de sustituir la instalación y se conserva respaldo fuera del escaneo. |
| Baja | Seguía apareciendo WebCapture Mobile en el nombre accesible de un contenedor. | Contenedor identificado como Joseador; metadatos y recursos nativos comprobados como 0.3.1 / PAPIELGUAPO. |

## Pruebas realizadas

- **CTest: 3/3 suites superadas.** Política de URL/estado (18 casos), validación de entrada nativa (dirección, eje, caducidad y rechazo de repetición) y credenciales sintéticas (guardar, leer y eliminar; HTTPS y origen principal).
- **JavaScript:** nueve rutas web y 36 combinaciones de flechas; exclusión de campos editables y de eventos sintéticos; autocompletado sin sobrescribir texto ya escrito.
- **CEF real:** recarga con una segunda carga del documento, recorrido Atrás hasta Inicio, ajuste de once accesos en viewport de 256 px y gestos táctiles confiables en ambos sentidos.
- **Audio:** señal controlada de 440 Hz recibida por el puente con RMS aproximado 0,125–0,127; captura WASAPI del árbol del navegador con RMS cero. Sin salida directa duplicada de Windows en esa prueba.
- **VST3:** instrumento, MIDI, estado, cero entradas y dos salidas, a 44,1/48/96 kHz.
- **Comprobación visual de la aplicación de escritorio:** el botón recargó un archivo local modificado; una página HTTP local recibió un gesto vertical con ↓ (desplazamiento observado de 236 px), un gesto horizontal con → (dx observado de −274 px) y otro gesto con la rueda. Su mensaje de consola sin entrada del usuario no produjo un gesto antes de la prueba.

Las pruebas de credenciales usaron exclusivamente una cuenta ficticia bajo webcapture-test.invalid. La prueba visual empleó contenido local sin cuentas ni datos personales. No se inició sesión en servicios reales.

## Límites y trabajo pendiente

- Cada web decide cómo responde a los gestos y si acepta navegadores integrados. No se ha certificado TikTok, Spotify, SoundCloud, Gmail o WhatsApp con cuentas reales ni todos los DAWs.
- El filtro de URLs reconoce parámetros comunes; no puede identificar todos los secretos arbitrarios incluidos en una dirección. No sustituye al gestor de credenciales.
- El autocompletado sigue siendo de un usuario por origen y para formularios compatibles. QR, passkeys y autenticación especial dependen del servicio.
- Las instancias simultáneas siguen asignando el primer perfil libre. Un orden de carga distinto puede asignar otro perfil; no se ha cambiado esa limitación.
- Se conserva el motor CEF fijado por el proyecto. Esta revisión no actualiza Chromium ni acredita que todas las dependencias estén libres de vulnerabilidades; su actualización y validación de códecs requieren un trabajo separado.
- No se ha realizado fuzzing, análisis de malware, prueba de penetración externa ni revisión completa del motor de terceros. La comprobación visual corresponde a la aplicación de escritorio que comparte el editor; no es una nueva certificación visual en FL Studio.

## Reproducir pruebas nativas

Con el proyecto configurado mediante CMake y WEBCAPTURE_BUILD_TESTS=ON:

```text
cmake --build build --config Release --target NavigationPolicyTest InputGateTest PasswordStoreTest
ctest --test-dir build -C Release --output-on-failure
```

También se incluyen BridgeProbe (opciones --reload-test, --history-test y --home-test), MobileTest y Tests/BrowserScripts.test.cjs. Consulta VALIDACION.md para el ensayo de persistencia.

[Volver a la guía](GUIA.md) · [Código y descargas](https://github.com/papielguapo/Joseador)

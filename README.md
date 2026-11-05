# Joseador 0.3.1

**Creador: PAPIELGUAPO.** Instrumento VST3 para Windows 11 x64 con navegador móvil reescalable y audio exclusivo al DAW. Anteriormente llamado WebCapture Mobile.

[**Descargar VST3**](https://github.com/papielguapo/Joseador/releases/latest) · [**Guía de instalación y uso**](docs/GUIA.md) · [**Auditoría 0.3.1**](docs/AUDITORIA.md)

<img src="docs/images/joseador.jpg" alt="Interfaz real de Joseador con pantalla móvil, once accesos web y buscador inferior" width="400">

*Interfaz real de 0.3.1: Inicio y botón Actualizar página, capturados en la aplicación de escritorio que comparte el editor VST3.*

## Instalar en FL Studio

Descarga el ZIP desde [Releases](https://github.com/papielguapo/Joseador/releases), extráelo y ejecuta install.ps1 con FL Studio cerrado. Conserva toda la carpeta Joseador.vst3, incluido BrowserRuntime.

En Options > Manage plugins, añade %LOCALAPPDATA%\Programs\Common\VST3 si es necesario y pulsa Find installed plugins. Carga Joseador, de PAPIELGUAPO, como instrumento. Asígnalo al insert del mezclador que quieras y graba con Edison o la grabación de pistas de FL.

El instalador conserva la versión anterior fuera del escaneo. Se mantienen los identificadores VST3 y el estado. Vuelve a escanear para actualizar el nombre en la caché del DAW.

## Uso

La salida al DAW está siempre activa. Las notas MIDI no disparan ni afinan la web. No necesita una nota sostenida. No hay grabador WAV, reinicio ni selector de escritorio. Al cerrar el editor sigue reproduciendo mientras el instrumento permanezca cargado y el host procese el canal. El transporte del DAW no controla la web y no hay renderizado acelerado sin conexión.

Inicio tiene cuatro columnas y once accesos: YouTube, TikTok, Instagram, Facebook, X, Reddit, Twitch, SoundCloud, Spotify, WhatsApp Web y Gmail web. Abren sus páginas dentro del plugin. La barra inferior acepta direcciones o búsquedas con Enter; el botón izquierdo retrocede por el historial hasta Inicio y el botón circular de la derecha actualiza la página. Se ha retirado el micrófono.

La rueda genera gestos táctiles en todas las webs, incluidas TikTok, Reels, Shorts, SoundCloud y Spotify. Con la página enfocada, ↑/↓ envían gestos verticales y ←/→ horizontales. En historias de Instagram/Facebook, bajar avanza y subir retrocede. Campos editables y comentarios desplazables conservan su manejo habitual. El resultado depende de cada web.

La ventana admite de 320 × 400 a 1200 × 1400, con viewport móvil adaptado al DPI. Inicio se ajusta sin desbordamiento. El marco recuerda a un iPhone; no emula iOS ni Safari. Hora, conexión y batería proceden del PC. Wi-Fi/LAN indica conexión del adaptador, no intensidad ni acceso garantizado a Internet.

## Sesión y contraseñas

Perfiles persistentes: %LOCALAPPDATA%\WebCaptureMobile\Profiles\SlotN. Cada instancia usa el primer perfil libre; las simultáneas usan perfiles separados. Al reiniciar se reutilizan. Si cambia el orden de carga de varias instancias puede cambiar su perfil. Los antiguos perfiles temporales pueden requerir iniciar sesión de nuevo.

Cookies y almacenamiento web persisten, sujetos a caducidad y controles del servicio. En formularios HTTPS compatibles, Joseador pregunta si quieres guardar o actualizar la contraseña. Usa el Administrador de credenciales de Windows, con un usuario por origen web, y puede rellenarla posteriormente. No escribe contraseñas en logs, URLs o localStorage. Puedes eliminar las entradas WebCaptureMobile:https://... en las credenciales genéricas de Windows.

QR, passkeys y formularios especiales dependen del servicio. No se ha probado el acceso con cuentas reales a Gmail o WhatsApp. Algunas webs rechazan navegadores integrados o redirigen desde la vista móvil.

## Audio y compatibilidad

CEF entrega PCM estéreo de 32 bits float sin reproducción local adicional de Windows. Se solicita la frecuencia del proyecto y se adapta otra frecuencia mediante interpolación sinc. La calidad depende del contenido original. Puede existir latencia respecto al transporte.

Instrumento/MIDI/estado comprobados a 44,1/48/96 kHz. FL Studio 2026 abrió una versión anterior de la interfaz. No está certificado con todos los DAWs o webs. Pro Tools no carga VST3 directamente. No soporta macOS, Linux ni hosts de 32 bits.

## Compilar

Requiere Visual Studio con C++ x64, CMake y Windows 11 SDK. Ejecuta build.ps1. Descarga JUCE 8.0.6 y CEF 150.0.10 / Chromium 150.0.7871.101 con H.264/AAC desde [godwinmix-cef](https://github.com/psmux/godwinmix-cef/releases/tag/cef-150.0.10), verifica el SHA-256 fijado y compila.

Resultado: build/WebCaptureMobile_artefacts/Release/VST3/Joseador.vst3. Nombres internos WebCapture se mantienen por compatibilidad de estado, perfiles y credenciales. El navegador usa otro proceso y el bootstrap CEF con sandbox.

## Licencias

Código propio MIT. JUCE mantiene licencia AGPLv3/comercial; CEF y demás componentes mantienen sus licencias. Consulta ThirdParty, incluidos los avisos de códecs, antes de redistribuir. VALIDACION.md describe las pruebas. Git contiene código y documentos; los binarios están en Releases o se generan al compilar.

La versión 0.3.1 refuerza el origen de las credenciales, la validación de gestos y las URLs del estado. Consulta [la auditoría interna y sus límites](docs/AUDITORIA.md).

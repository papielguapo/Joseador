# Validación de Joseador 0.3.0

Windows 11 x64, MSVC, SDK 10.0.26100, JUCE 8.0.6, CEF API 15000 y motor CEF 150.0.10 con H.264/AAC.

- Manifiesto y recursos: Joseador, PAPIELGUAPO, 0.3.0, Instrument/Synth. Identificadores VST3 conservados.
- Instrumento descubierto, MIDI aceptado, cero entradas y dos salidas; procesamiento a 44,1/48/96 kHz y restauración del estado.
- Señal Web Audio de 440 Hz: RMS del puente aproximadamente 0,125–0,127; captura WASAPI independiente del navegador con RMS cero. Sin escucha duplicada en Windows.
- Soporte MP4 H.264/AAC declarado probably.
- Página controlada recibió touchstart/touchmove con isTrusted=true en ambos sentidos: WEBCAPTURE_TOUCH:yes. Usa el generador táctil de la rueda sin mover el ratón del usuario.
- Nueve rutas JavaScript y 36 casos de flechas comprobaron TikTok, Reels, historias, Shorts, SoundCloud, Spotify y sitios ajenos. Autocompletado/guardado, preservación del texto ya escrito y exclusión de HTTP comprobados.
- Inicio: 11 enlaces, incluidos WhatsApp Web y Gmail. WEBCAPTURE_FIT:yes en viewport de 256 px.
- Dos ejecuciones con cierre entre ellas conservaron una cookie de sesión y localStorage de un servidor HTTP local en perfil aislado: WEBCAPTURE_PERSIST:yes.
- Credencial sintética guardada, leída y eliminada bajo https://webcapture-test.invalid; restricción HTTPS comprobada. No se consultaron credenciales reales.

Fuentes: Tests/Smoke.cpp, BridgeProbe.cpp, BrowserScripts.test.cjs, ProfileFixture.py y PasswordStoreTest.cpp. Para persistencia, ejecutar el servidor local y BridgeProbe con --profile-test=write y después --profile-test=read.

La revisión visual previa en FL Studio 2026 confirmó Synth x64 y YouTube móvil en una interfaz anterior. No se ha revisado visualmente Joseador 0.3.0 dentro de FL. Los ensayos controlados no acreditan gestos o autenticación con cuentas reales de TikTok, Instagram, WhatsApp o Gmail, ni certifican todos los DAWs.

- Historial CEF con navegación activada por usuario: dos pasos atrás llegaron a Inicio y una tercera pulsación permaneció allí (HISTORY:yes). El buscador muestra la dirección actual al perder el foco.
- VST3 recompilado con botón Atrás/Inicio y sin micrófono; campo ampliado.

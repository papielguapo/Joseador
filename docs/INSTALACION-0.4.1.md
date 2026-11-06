# Instalación de JOSEADOR 0.4.1

El instalador EXE y el script alternativo instalan el paquete completo en la carpeta VST3 compartida de Windows x64: normalmente `C:\Program Files\Common Files\VST3\JOSEADOR.vst3`. El EXE obtiene la ubicación de Windows mediante `FOLDERID_ProgramFilesCommonX64`, sin confundirla con ProgramData ni con Common Files x86. Windows pide permisos de administrador.

No es un parche: incluye JOSEADOR 0.4.1, BrowserRuntime completo y el instalador oficial de Visual C++ x64. No descarga ni instala JOSEADOR 0.4.0 o WebCapture; tampoco necesita sus archivos. Una instalación ya existente en el destino compartido se respalda antes de sustituirla. Los respaldos quedan fuera de las rutas de plugins, en `%PROGRAMDATA%\JOSEADOR\PreviousVersions`.

Los perfiles web personales permanecen en `%LOCALAPPDATA%\WebCaptureMobile\Profiles`. Una copia antigua instalada en la ruta de usuario no se usa para instalar la nueva. Para evitar que el DAW encuentre dos copias, retira la anterior de esa ruta con el DAW cerrado, tal como explica la guía.

## Comprobaciones

- Compilados EXE y VST3; ambos declaran versión 0.4.1.
- Seis suites CTest aprobadas, incluida la instalación en un destino vacío, actualización, respaldo, restauración ante fallo y rechazo de un paquete incompleto.
- Host VST3: instrumento, MIDI y estado a 44,1/48/96 kHz. Audio CEF recibido por el instrumento; RMS de salida directa a Windows igual a cero.
- El manifiesto del EXE requiere administrador para escribir en Common Files.

Esta revisión no incluye una instalación interactiva en un Windows limpio ni una nueva certificación en todos los DAWs. La funcionalidad del navegador y del chat conserva la [auditoría de 0.4.0](AUDITORIA-0.4.0.md).

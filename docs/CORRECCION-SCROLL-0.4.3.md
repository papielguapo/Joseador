# Regresión de scroll corregida en 0.4.3

En 0.4.2 se retiró la conversión nativa de la rueda para delegarla en el evento wheel del documento. Con la emulación móvil esa entrega no es fiable: se podía recibir entrada de ratón sin producir el gesto táctil. La prueba anterior empezaba directamente en la función del gesto y no detectaba ese fallo de conexión.

Se restaura la conversión en el manejador nativo, conservando los límites entre gestos y la cancelación al navegar. El modo se actualiza según el elemento real bajo las últimas coordenadas del cursor, incluso si la web cambia el DOM sin mover el ratón. Los comentarios desplazables y campos pueden conservar su comportamiento normal. El evento original se consume cuando se convierte, para evitar un segundo gesto por JavaScript.

## Regresión reproducida y verificación

BridgeProbe incorpora `--joseador-native-input-test`. En un documento local controlado prueba el manejador de rueda en ambos sentidos y el recorrido de las flechas Arriba/Abajo hasta los gestos táctiles de CEF. Se exige que los cuatro gestos lleguen con eventos confiables y desplazamiento en el sentido esperado.

Se compiló el mismo arnés con el manejador de ratón de 0.4.2: `WEBCAPTURE_NATIVE_INPUT:no`, salida 14. Con el manejador corregido: `WEBCAPTURE_NATIVE_INPUT:yes`, salida 0. Esta prueba llama al manejador con argumentos de rueda e inyecta las flechas en CEF; no equivale a una certificación de todos los ratones y DAWs ni a una prueba manual larga de TikTok.

También pasan las siete suites CTest, los nueve recorridos web y 36 combinaciones de flechas, los casos de comentarios bajo un cursor inmóvil y las pruebas VST3 a 44,1/48/96 kHz. En el ensayo de audio, la salida directa del navegador a Windows tiene RMS cero.

Se mantienen los demás cambios de 0.4.2: Inicio sin franja, Wi-Fi con X, pantalla sin Internet con imagen integrada, retorno a Inicio al reconectar y Enter con inicio/campo vacío. El error de reproducción de algunos vídeos de TikTok sigue separado de esta corrección de entrada.

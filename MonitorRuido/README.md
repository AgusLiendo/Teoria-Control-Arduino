# Ajuste del muestreo ADC para evitar desconexiones Wi-Fi

**Cambio de diagnostico:** la documentacion oficial de ESP8266 Arduino advierte que llamar a `analogRead()` demasiadas veces puede hacer que Wi-Fi deje de funcionar. En la version anterior `SensorRuido.cpp` ejecutaba un `while` de lectura intensiva durante 50 ms, de forma repetida. Esta version muestrea sin bloquear, usando una sola lectura cada 5 ms (`INTERVALO_MUESTREO_ADC_US` en `Config.h`), y mantiene ventanas de 50 ms agrupadas de a 20.

**Importante para el informe:** al reducir la tasa de muestreo, la amplitud pico a pico del sonido puede estar fuertemente submuestreada y los dB estimados pueden resultar poco fiables. No usar este cambio como calibracion definitiva del sistema acustico. La prueba actual es de estabilidad Wi-Fi. Si se requiere caracterizar con mayor fidelidad senales acusticas, evaluar un circuito detector de envolvente o un microcontrolador/ADC adecuado para muestreo de audio con Wi-Fi activo.

**Prueba sugerida:** habilitar `HABILITAR_PANEL_WEB=true`, `HABILITAR_SENSOR=true`, `HABILITAR_CONTROL=false`, `HABILITAR_TELEGRAM=false`, y `SERVO_HABILITADO=0`. Probar 10-15 minutos con KY-037 conectado y observar que no aparezcan errores Wi-Fi 200/201. Luego evaluar las lecturas, no antes.

---

# Diagnostico por modulos (nuevo)

Configuraciones en `Config.h` (`true` = activo, `false` = inactivo):

| Prueba | PANEL_WEB | SENSOR | CONTROL | TELEGRAM |
|---|---|---|---|---|
| A: Wi-Fi + panel solamente | `true` | `false` | `false` | `false` |
| B: agregar lectura KY-037 | `true` | `true` | `false` | `false` |
| C: agregar control y buzzer | `true` | `true` | `true` | `false` |
| D: agregar Telegram | `true` | `true` | `true` | `true` |

El paquete viene configurado para **prueba A**. Con el sensor desactivado el panel muestra `0` dB y NORMAL, lo cual es esperado. Durante A desconectar KY-037 y buzzer con el USB apagado; para B reconectar KY-037; para C conectar también el buzzer. Mantener el servo desconectado (`SERVO_HABILITADO 0`). Reiniciar/cargar el programa cada vez que se modifique un interruptor.

Cuando `HABILITAR_TELEGRAM=false`, ni los avisos automáticos ni el botón de prueba intentarán enviar mensajes por HTTPS; tampoco se inicia NTP. El botón puede mostrar un error esperado. Con `true` se reactivan ambos, sujeto a credenciales válidas.

La estabilidad se compara usando la misma red, USB y placa por 10-15 minutos en cada etapa. Mantener las credenciales privadas y colocar el token renovado, nunca el token expuesto anteriormente.

---

# Monitor de ruido - NodeMCU ESP8266

Proyecto dividido en archivos para Arduino IDE. Cumple el diseño del informe *TP_TEORIA_DEL_CONTROL*: control on/off con histéresis; panel HTTP local; fallback AP; Telegram solo emisor fuera de horario.

## Primer uso

1. Instalá desde Arduino IDE el paquete de placas **esp8266 by ESP8266 Community** y seleccioná **NodeMCU 1.0 (ESP-12E Module)**.
2. Descomprimí la carpeta `MonitorRuido` completa y abrí `MonitorRuido.ino`. **No abras los `.cpp` por separado:** Arduino IDE los compila automáticamente si están en la misma carpeta del sketch.
3. Editá `Config.h`: `WIFI_SSID`, `WIFI_PASSWORD`, `TELEGRAM_BOT_TOKEN`, `TELEGRAM_CHAT_ID`, `PANEL_CLAVE` y `AP_CLAVE`. BotFather proporciona el token. Enviale `/start` al bot y consultá `https://api.telegram.org/bot<TOKEN>/getUpdates` para obtener `chat.id`. No publiques estas credenciales.
4. Conectá **KY-037**: `AO -> A0`, `+ -> 3V3`, `G -> GND` (no uses DO). **Buzzer KY-012**: `S -> D6`, `- -> GND` (si tiene pin `+`, conectalo a la tensión indicada por su módulo). Si el buzzer no funciona, la lectura y el panel no dependen de él. El informe usa D1; se preserva D6 porque esa es la conexión actual de las pruebas.
5. **Servo no conectado**: `SERVO_HABILITADO 0`. Cuando tengas fuente de 5 V / >=1 A y GND común, la señal va a D2 y se puede activar en `Config.h` junto con una calibración mecánica de ángulos.
6. Subí el proyecto y abrí el monitor serie a **115200 baudios**. En STA aparecerá `http://<IP-DE-LA-PLACA>`; si Wi-Fi no conecta en 10 s, conectate a la red `MonitorRuido` y abrí `http://192.168.4.1`.
7. El panel muestra nivel, estado, puerta (orden simulada), gráfico en vivo, configuración y botón **Probar Telegram**. Las acciones de configuración/prueba piden autenticación HTTP Basic (`PANEL_USUARIO` y `PANEL_CLAVE`).

## Valores iniciales (configurables desde el panel)

| Parámetro | Real (predeterminado) | Prueba sugerida |
|---|---:|---:|
| Umbral alto | 65 dB estimados | 65 dB estimados |
| Histéresis | 3 dB | 3 dB |
| Offset | 20 dB **sin calibrar** | calibrar |
| Cierre | 180 s | 10 s |
| Reapertura | 120 s | 5 s |
| Tolerancia a pausas | 10 s | 2 s |
| Preaviso buzzer | 5 s | 5 s |
| Aviso Telegram fuera de horario | 60 s | 10 s |
| Enfriamiento Telegram | 300 s | 300 s |
| Horario habitual | 07:00-22:00, lunes a viernes | igual |

**Nota de medición:** el KY-037 no entrega dB(A). Se calcula la amplitud pico a pico en ventanas de 50 ms; se promedian energéticamente 20 ventanas (1 s) y se añade el offset. Calibrar con Wi-Fi encendido contra un sonómetro de referencia antes de usar umbrales expresados en dB. No modificar el potenciómetro luego de calibrar.

## Estructura

- `Config.h`: secretos, red, pines y habilitación del servo.
- `Estado.h` / `Estado.cpp`: parámetros compartidos y estado actual.
- `SensorRuido.h` / `SensorRuido.cpp`: medición y estimación logarítmica.
- `Control.h` / `Control.cpp`: NORMAL/PRECAUCION/CERRADA, histéresis, buzzer y servo opcional.
- `Telegram.h` / `Telegram.cpp`: NTP, horario inusual, alarmas y HTTP POST a sendMessage.
- `PanelWeb.h` / `PanelWeb.cpp`: rutas HTTP, autenticación y validación.
- `PanelHtml.h`: interfaz HTML/CSS/JavaScript embebida en la memoria flash.
- `MonitorRuido.ino`: inicialización, red, AP de respaldo y ciclo principal.

## API

- `GET /` panel público en red local.
- `GET /api/estado` estado JSON público en red local.
- `POST /api/config` ajustes y modo manual; requiere HTTP Basic.
- `POST /api/prueba-telegram` mensaje de prueba; requiere HTTP Basic.

Los cambios de configuración se guardan **en RAM únicamente**; al reiniciar vuelven los predeterminados de `Estado.h`. El gráfico mantiene las últimas 60 lecturas **solo en el navegador**.

## Seguridad y limitaciones

- Es un **prototipo de laboratorio**. El panel usa HTTP sin cifrar; contraseña Basic sin protección criptográfica. Usarlo solamente en una red local confiable, cambiar todas las claves y **no publicar puertos a Internet**.
- De acuerdo con la limitación documentada del informe, la conexión HTTPS a Telegram está cifrada pero **no valida el certificado del servidor** (`setInsecure()`). Para uso real se debe usar un almacén de CA y validación TLS.
- Telegram requiere Internet, token y chat ID; las alertas automáticas también requieren hora NTP sincronizada y horario inusual. Un mensaje de prueba puede enviarse sin NTP si hay Internet.
- Al caer el Wi-Fi se intenta reconectar; en arranque sin router se habilita AP de respaldo y se mantiene STA intentando reconectar. La puerta y el buzzer funcionan independientemente de Internet.
- El envío HTTPS puede pausar por varios segundos el ciclo del control; el prototipo tolera ese retardo. Para un dispositivo instalado hay que diseñar y verificar temporización y seguridad adicional.
- La orden manual sobre la puerta es una **simulación lógica** hasta habilitar físicamente el servo; no se afirma que haya movimiento real.

## Diagnostico Wi-Fi (version actualizada)

- En `MonitorRuido.ino`, `registrarEventosWiFi()` registra las desconexiones y muestra el codigo numerico de `evento.reason`.
- En `iniciarRed()` se utiliza `WiFi.setSleepMode(WIFI_NONE_SLEEP)` para descartar problemas de ahorro de energia.
- Cada 5 segundos se imprime el estado real de la conexion; se puede desactivar con `DIAGNOSTICO_WIFI_HABILITADO = false` en `Config.h`.
- Si la placa se desconecta, la IP que imprimio inicialmente puede dejar de ser valida. Comprobar el estado actual antes de acceder al panel.

## Ajuste de dB desde Config.h

- `UMBRAL_RUIDO_DB`: umbral alto de alerta y de entrada al estado PRECAUCION (65.0f por defecto).
- `OFFSET_CALIBRACION_DB`: correccion para convertir amplitud analogica en dB estimados (20.0f inicial, a calibrar).
- Estos son los **valores de arranque**. La modificacion desde el panel prevalece mientras la placa esta encendida, pero no se guarda al reiniciar.

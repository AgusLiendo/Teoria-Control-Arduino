# Monitor de ruido y control automático de puerta — ESP8266

Prototipo desarrollado para la asignatura **Tecnologías para la Automatización**. Monitorea el ruido de un pasillo mediante un micrófono, aplica un control **on-off con histéresis y temporización** y acciona un servomotor para abrir o cerrar una puerta de maqueta. Incluye un panel web local para supervisión y configuración, y avisos por Telegram ante ruido persistente fuera del horario habitual.

El proyecto utiliza una **NodeMCU ESP8266**, un módulo de micrófono analógico, un buzzer y un servomotor. La medición del pasillo permite actuar anticipadamente sobre la perturbación (control por **prealimentación**); no mide el ruido dentro del aula ni verifica directamente la atenuación lograda.

> **Alcance:** prototipo académico, no sistema de seguridad ni controlador apto para una puerta real. El nivel mostrado en dB es una **estimación relativa que requiere calibración**; no corresponde a una lectura normalizada en dB(A).

## Funcionalidades

- Medición periódica de amplitud analógica y estimación logarítmica del nivel de ruido.
- Promedio cuadrático de las amplitudes obtenidas en ventanas de medición.
- Máquina de estados `NORMAL`, `PRECAUCION` y `CERRADA` con histéresis, tolerancia a pausas y tiempos configurables.
- Preaviso intermitente mediante buzzer antes del cierre automático.
- Control de apertura/cierre por servomotor, con desplazamiento gradual cuando está habilitado.
- Modos de puerta **automático**, **forzar abierta** y **forzar cerrada** desde el panel web.
- Interfaz web local con nivel estimado, estado, orden de puerta, parámetros e historial gráfico de las últimas 60 mediciones del navegador.
- Conexión WiFi en modo estación (STA) y creación de un punto de acceso (AP) de respaldo si falla la conexión inicial.
- Alertas por Telegram ante ruido sostenido fuera del horario habitual, con tiempo mínimo entre avisos.
- Activación individual de módulos y diagnóstico de conectividad WiFi.

## Hardware y conexiones

| Componente | Conexión | Observaciones |
| --- | --- | --- |
| NodeMCU ESP8266 | USB a la computadora / alimentación apropiada | Placa controladora |
| Micrófono analógico KY-037 | `AO → A0`, `+ → 3V3`, `G → GND` | Se utiliza la salida analógica, no `DO` |
| Buzzer | Señal `→ D6`, GND `→ GND` | Verificar el tipo y los requisitos eléctricos del buzzer utilizado |
| Servomotor | Señal `→ D2` (`GPIO4`) | Los ángulos se calibran según el montaje mecánico |
| Alimentación del servo | Fuente externa regulada de 5 V, dimensionada para el servo | **Unir el GND de la fuente externa con el GND de la NodeMCU** |

**Precauciones:** no alimentar el servo desde el pin `3V3` del ESP8266; evitar forzarlo contra topes mecánicos. Verificar que ninguna entrada del ESP8266 reciba tensiones fuera de su rango admisible. En caso de fallas eléctricas o reinicios, revisar corriente disponible, masa común y desacoplamiento de la fuente. Una puerta real requiere mecanismos y protecciones adicionales.

El firmware contiene control de servo real en `Control.cpp`, condicionado por `SERVO_HABILITADO`. Cuando está habilitado, envía órdenes de posición al servo y avanza hacia el ángulo objetivo en pasos de 2° cada 20 ms. **El panel presenta la posición ordenada**, no una confirmación física de que la puerta se haya movido o cerrado.

## Estructura del proyecto

```text
MonitorRuido/
├── MonitorRuido.ino       # Inicialización, WiFi y ciclo principal
├── Config.h              # Configuración local privada (no versionada)
├── Estado.h / Estado.cpp # Parámetros y estado compartido
├── SensorRuido.h / .cpp  # Lectura ADC y cálculo del nivel estimado
├── Control.h / .cpp      # Máquina de estados, buzzer y servomotor
├── Telegram.h / .cpp     # Horarios, NTP y notificaciones
├── PanelWeb.h / .cpp     # Servidor HTTP, API y autenticación
├── PanelHtml.h           # HTML, CSS y JavaScript del panel
└── README.md             # Esta guía
```

`Config.h` está excluido del repositorio mediante `.gitignore` porque puede incluir credenciales privadas. **Es necesario crearlo o disponer de una copia local compatible antes de compilar.** Aún no se publica un archivo `Config.example.h`; por lo tanto, el repositorio por sí solo no ofrece una configuración de compilación completa.

## Instalación y puesta en marcha

1. Instalar **Arduino IDE** y el paquete de placas **esp8266 by ESP8266 Community**. Seleccionar **NodeMCU 1.0 (ESP-12E Module)** o la variante correspondiente al hardware.
2. Descargar el repositorio y abrir `MonitorRuido/MonitorRuido.ino` desde Arduino IDE. Los archivos `.cpp` y `.h` de la misma carpeta forman parte del sketch.
3. Crear o recuperar el archivo local `MonitorRuido/Config.h` con las constantes utilizadas por el firmware. Configurar la red WiFi, usuario y clave del panel, AP de respaldo, credenciales de Telegram, pines, parámetros y banderas de módulos.
4. Realizar las conexiones con la alimentación desconectada. Para utilizar el servomotor, conectar una **fuente externa de 5 V adecuada**, con GND común, y habilitarlo en la configuración.
5. Compilar y cargar el programa. Abrir el **Monitor Serie a 115200 baudios**.
6. Si la NodeMCU se conecta al router, el monitor mostrará la IP del panel (`http://IP-DE-LA-PLACA`). Si la conexión inicial falla durante unos 10 segundos, intentará activar un AP de respaldo, accesible normalmente en `http://192.168.4.1` al conectarse a su red.
7. Abrir el panel desde un dispositivo en la red correspondiente y verificar la lectura, el estado del control y las órdenes de puerta. Las rutas de escritura requieren autenticación HTTP Basic.

**Importante:** el estado de las banderas de módulos y los valores predeterminados dependen del `Config.h` local. No se presupone que el servo o Telegram estén habilitados en todas las instalaciones.

### Configuración relevante

Las constantes de `Config.h` que utiliza el código incluyen:

| Grupo | Identificadores |
| --- | --- |
| Red y panel | `WIFI_SSID`, `WIFI_PASSWORD`, `AP_NOMBRE`, `AP_CLAVE`, `PANEL_USUARIO`, `PANEL_CLAVE` |
| Telegram | `TELEGRAM_BOT_TOKEN`, `TELEGRAM_CHAT_ID`, `HABILITAR_TELEGRAM` |
| Módulos | `HABILITAR_PANEL_WEB`, `HABILITAR_SENSOR`, `HABILITAR_CONTROL`, `SERVO_HABILITADO` |
| Pines | `PIN_MICROFONO`, `PIN_BUZZER`, `PIN_SERVO` |
| Medición | `INTERVALO_MUESTREO_ADC_US`, `VENTANA_MS`, `VENTANAS_POR_PROMEDIO`, `OFFSET_CALIBRACION_DB` |
| Control | `UMBRAL_RUIDO_DB`, `HISTERESIS_DB`, `TIEMPO_CIERRE_S`, `TIEMPO_REAPERTURA_S`, `TOLERANCIA_PAUSAS_S`, `PREAVISO_BUZZER_S` |
| Servo | `ANGULO_PUERTA_ABIERTA`, `ANGULO_PUERTA_CERRADA` |
| Avisos y horarios | `TIEMPO_AVISO_TELEGRAM_S`, `ESPERA_ENTRE_ALERTAS_S`, `HORA_INICIO_HABITUAL`, `HORA_FIN_HABITUAL`, `ACTIVIDAD_FIN_SEMANA` |
| Otros | `BUZZER_SILENCIADO`, `DIAGNOSTICO_WIFI_HABILITADO`, `INTERVALO_DIAGNOSTICO_WIFI_MS` |

Los ángulos del servo dependen del sentido de montaje: **no es obligatorio que abierto sea 0° y cerrado 90°**. Ajustarlos evitando esfuerzos o golpes. Las banderas `HABILITAR_*` permiten hacer pruebas por etapas.

### Parámetros modificables desde el panel

El formulario permite cambiar umbral alto, histéresis, offset, tiempos de cierre, reapertura y tolerancia, duración del preaviso, tiempo para avisar por Telegram, enfriamiento entre alertas, horario habitual, actividad de fin de semana, silencio del buzzer y modo de puerta.

Los cambios se aplican a la **RAM**, sin persistencia: al reiniciar se recuperan los valores de arranque definidos en `Config.h` / `Estado.h`. La aplicación comprueba rangos e incompatibilidades básicas. Los valores que se muestran en el panel son los efectivos en esa sesión.

## Funcionamiento del control

El nivel alto es `umbralAltoDb` y el bajo se calcula como `umbralAltoDb - histeresisDb`.

| Estado | Comportamiento |
| --- | --- |
| `NORMAL` | Orden de puerta abierta en modo automático; al alcanzar el umbral alto pasa a `PRECAUCION`. |
| `PRECAUCION` | Inicia el temporizador de cierre, tolera pausas por debajo del umbral bajo menores que `toleranciaMs` y activa el buzzer intermitente durante los últimos segundos. Si la calma supera la tolerancia, vuelve a `NORMAL`; si vence el temporizador, pasa a `CERRADA`. |
| `CERRADA` | Ordena cerrar la puerta; una vez acumulado un período continuo de calma por debajo del umbral bajo de `tiempoReaperturaMs`, vuelve a `NORMAL`. |

El temporizador de `PRECAUCION` **no mide exclusivamente segundos por encima del umbral**: continúa mientras no se abandone ese estado, incluso durante pausas breves toleradas. En modo manual se fuerza la orden al servo, sin sustituir la lógica interna de evaluación del ruido. El preaviso del buzzer está vinculado al estado `PRECAUCION`, no a una orden manual de cierre.

## Medición y calibración

`SensorRuido.cpp` toma lecturas ADC no bloqueantes separadas por al menos `INTERVALO_MUESTREO_ADC_US`. Para cada ventana de `VENTANA_MS`, calcula `amplitud = máximo - mínimo`. Después de `VENTANAS_POR_PROMEDIO` ventanas calcula:

```text
amplitud_efectiva = sqrt(promedio(amplitud_ventana²))
nivel_estimado = 20 × log10(max(amplitud_efectiva, 1)) + offsetDb
```

La configuración descrita en el informe utiliza aproximadamente **una lectura cada 5 ms**, ventanas de **50 ms** y **20 ventanas por promedio** (aproximadamente 1 s). Este muestreo se adoptó para reducir la carga del ADC y los problemas de estabilidad del WiFi detectados durante el desarrollo.

**Limitación fundamental:** una lectura cada 5 ms equivale aproximadamente a 200 muestras/s, insuficientes para reconstruir gran parte del sonido audible. Por eso el máximo y mínimo capturados pueden subestimar considerablemente la amplitud real, y el nivel derivado puede variar con la frecuencia y el tipo de sonido. El cálculo tampoco implementa ponderación A ni mide presión sonora RMS; **no debe describirse como un sonómetro calibrado ni como una medición Leq normalizada**.

Para evaluar el sistema, comparar las lecturas con un instrumento de referencia manteniendo el WiFi activo y la posición del sensor fija. Ajustar `offsetDb` y registrar errores para distintas condiciones de ruido. Una coincidencia puntual tras ajustar el offset **no demuestra exactitud en todo el rango**. Una futura mejora es usar un detector de envolvente o un sistema de adquisición adecuado para audio.

## Panel web y API

El panel muestra el nivel estimado, la máquina de estados, el modo de la puerta, la **orden** de apertura/cierre, la conexión WiFi, la hora y un gráfico temporal. Este gráfico conserva hasta 60 muestras **en el navegador**, sin almacenamiento permanente ni exportación de datos.

| Método y ruta | Función | Acceso |
| --- | --- | --- |
| `GET /` | Panel web | Sin autenticación |
| `GET /api/estado` | Estado y configuración actual en JSON | Sin autenticación |
| `POST /api/config` | Cambiar parámetros y modo | HTTP Basic |
| `POST /api/prueba-telegram` | Enviar un mensaje de prueba | HTTP Basic |

El panel funciona mediante HTTP en la red local o en el AP de respaldo. No se incluye una API de historial ni de exportación CSV.

## Alertas por Telegram

El firmware envía mensajes mediante la API `sendMessage` de Telegram. Para habilitarlo es necesario configurar un bot con **@BotFather**, obtener su token, iniciar una conversación con el bot y configurar el `TELEGRAM_CHAT_ID` correspondiente.

La alerta automática se considera cuando:

1. Se detecta un evento de ruido que alcanza el umbral alto y permanece activo el tiempo `TIEMPO_AVISO_TELEGRAM_S` (admitiendo pausas breves según tolerancia e histéresis).
2. La hora local está sincronizada mediante NTP y el evento ocurre fuera del horario habitual configurado, o en un fin de semana sin actividad habilitada.
3. Hay conexión a Internet y transcurrió el período de enfriamiento desde el último mensaje enviado; además se limita la frecuencia de reintentos.

Los mensajes incluyen nivel estimado, fecha/hora, duración del evento y orden de puerta. Si no hay hora sincronizada, **no se envían alertas automáticas**. El botón de prueba permite verificar el envío manual cuando hay conectividad y Telegram está habilitado.

Telegram es **solo emisor**: no recibe comandos ni abre/cierra la puerta a distancia.

## Diagnóstico y pruebas

Para localizar problemas, conviene habilitar los módulos de manera progresiva desde `Config.h`:

| Etapa | Panel | Sensor | Control/buzzer | Telegram |
| --- | --- | --- | --- | --- |
| A. Conectividad y panel | Sí | No | No | No |
| B. Lectura de micrófono | Sí | Sí | No | No |
| C. Lógica y actuadores | Sí | Sí | Sí | No |
| D. Integración completa | Sí | Sí | Sí | Sí |

En la etapa C, conectar y habilitar el servo solamente si está disponible su alimentación externa con GND común. Con el sensor deshabilitado, los valores del panel no representan mediciones reales. Cambios de banderas y constantes en `Config.h` requieren recompilar y volver a cargar el firmware.

Verificaciones recomendadas:

- **Sensor:** comprobar amplitudes y niveles en el Monitor Serie con distintas fuentes sonoras.
- **Control:** provocar transiciones entre `NORMAL`, `PRECAUCION` y `CERRADA`, incluyendo pausas breves y calma prolongada.
- **Servo:** verificar apertura/cierre físico, ángulos y ausencia de esfuerzo mecánico.
- **Buzzer:** comprobar el preaviso y la función de silencio.
- **Red:** probar tanto conexión STA como el punto de acceso de respaldo.
- **Panel:** verificar refresco, validaciones, modos manuales y pérdida de los ajustes al reiniciar.
- **Telegram:** probar envío manual y alertas fuera de horario, con y sin Internet o sincronización horaria.

El firmware registra eventos de desconexión WiFi, informa parámetros de red mediante el diagnóstico serie y utiliza `WIFI_NONE_SLEEP` para evitar el ahorro de energía durante las pruebas. No interpretar un resultado exitoso de conexión como validación de precisión acústica.

## Seguridad y limitaciones

- El panel usa **HTTP** sin cifrado y autenticación **Basic** para operaciones de escritura. Usarlo únicamente en redes confiables y **no exponer el puerto 80 a Internet**.
- `GET /` y `GET /api/estado` son públicos dentro de la red donde está accesible el dispositivo.
- La conexión HTTPS hacia Telegram emplea `BearSSL::WiFiClientSecure` con `setInsecure()`: **cifra el transporte, pero no valida el certificado remoto**. Es una limitación deliberada del prototipo que debe corregirse antes de un despliegue real.
- No subir `Config.h`, contraseñas, tokens ni identificadores privados. Si un token se publica accidentalmente, revocarlo y generar uno nuevo.
- Las operaciones HTTPS pueden bloquear temporalmente el ciclo principal; no hay garantías de tiempo real estricto.
- El módulo KY-037 y el método de muestreo empleado no constituyen un instrumento de medición acústica normalizado.
- No hay realimentación del ruido interior ni sensor que confirme la posición física de la puerta.
- No se almacenan datos históricos, ni parámetros en memoria no volátil.
- El sistema está destinado a una **maqueta**, no a una puerta de uso humano sin mecanismos de seguridad específicos.

## Documentación del proyecto

El diseño, fundamentos de control por prealimentación, selección de componentes, metodología de ensayo y resultados experimentales se desarrollan en el informe del Trabajo Práctico Integrador Final.

La validación final del prototipo debe incluir los resultados de calibración, pruebas de transición de estados, registro del accionamiento del servo, pruebas de conectividad y **mediciones reales de atenuación** con la puerta abierta y cerrada. No se presuponen resultados numéricos que todavía no estén documentados.

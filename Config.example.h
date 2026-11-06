#pragma once

// ===== AJUSTES DE RED =====
// Configuración de red WI-FI Primaria.
#define WIFI_SSID "TU_WIFI"
#define WIFI_PASSWORD "TU_CLAVE_WIFI"

//Ajustes Bot Telegram
#define TELEGRAM_BOT_TOKEN "API_BOT"
#define TELEGRAM_CHAT_ID "ID_CANAL"

// Autenticacion para las operaciones de escritura del panel.
#define PANEL_USUARIO "USUARIO_ADMIN"
#define PANEL_CLAVE "CONTRASENA_ADMIN"

// Red local de respaldo.
#define AP_NOMBRE "TU_AP"
#define AP_CLAVE "TU_CONTRASENA_AP"  // 8 caracteres como minimo



// ===== AJUSTES DEL NIVEL SONORO =====
// Umbral que dispara PRECAUCION y, si se sostiene, las alertas
constexpr float UMBRAL_RUIDO_DB = 20.0f;

// Calibracion de la estimacion del KY-037: dB = 20*log10(amplitud) + offset.
constexpr float OFFSET_CALIBRACION_DB = 5.0f;

//Histeresis (Diferencia entre el umbral alto y el nivel necesario para considerar que bajo el ruido.)
constexpr float HISTERESIS_DB = 3.0f;



// ===== AJUSTES DE TIEMPO DE RESPUESTA =====
// --- Control de la puerta y buzzer (segundos) ---
// Tiempo desde la entrada en PRECAUCION hasta ordenar el cierre si no se vuelve a NORMAL.
constexpr unsigned long TIEMPO_CIERRE_S = 20UL;

// Tiempo continuo de calma requerido para volver a NORMAL cuando la puerta esta CERRADA.
constexpr unsigned long TIEMPO_REAPERTURA_S = 20UL;

// Tiempo de calma tolerado durante PRECAUCION antes de cancelar la alerta y volver a NORMAL.
constexpr unsigned long TOLERANCIA_PAUSAS_S = 10UL;

// Duracion del aviso sonoro intermitente justo antes de que venza el tiempo de cierre.
constexpr unsigned long PREAVISO_BUZZER_S = 5UL;

// --- Telegram (segundos) ---
// Tiempo de ruido sostenido fuera del horario habitual necesario para generar una alerta.
constexpr unsigned long TIEMPO_AVISO_TELEGRAM_S = 60UL;

// Tiempo minimo de espera entre alertas de Telegram para evitar mensajes repetidos.
constexpr unsigned long ESPERA_ENTRE_ALERTAS_S = 60UL;



// ===== AJUSTES DE HORARIO =====
// --- Horario habitual ---
// Hora de inicio de la franja habitual (formato de 24 horas; 7 = 07:00).
constexpr unsigned int HORA_INICIO_HABITUAL = 7;

// Hora de finalizacion de la franja habitual (22 = 22:00).
// Fuera de esta franja pueden enviarse las alertas de Telegram.
constexpr unsigned int HORA_FIN_HABITUAL = 22;

// true: sabados y domingos cuentan como horario habitual; false: quedan fuera.
constexpr bool ACTIVIDAD_FIN_SEMANA = false;

// ===== OTROS AJUSTES =====
// true: no sonar el buzzer durante el preaviso; false: habilitarlo.
constexpr bool BUZZER_SILENCIADO = false;

// El modo de la puerta NO se agrega aca: se inicia en AUTOMATICO
// y se cambia desde el panel web segun se necesite.



// ===== MODULOS ACTIVABLES PARA DIAGNOSTICO =====
// True habilita false deshabilita
constexpr bool HABILITAR_PANEL_WEB = true;
constexpr bool HABILITAR_SENSOR = true;
constexpr bool HABILITAR_CONTROL = true;
constexpr bool HABILITAR_TELEGRAM = true;
constexpr bool DIAGNOSTICO_WIFI_HABILITADO = true;
constexpr unsigned long INTERVALO_DIAGNOSTICO_WIFI_MS = 5000UL;
// 1 habilita 0 deshabilita
#define SERVO_HABILITADO 1



// ===== AJUSTE DE PINES =====
constexpr unsigned int PIN_MICROFONO = A0;
constexpr unsigned int PIN_BUZZER = D6;
constexpr unsigned int PIN_SERVO = D2; // Solo cuando SERVO_HABILITADO = 1



// ===== AJUSTES DE SERVO =====
constexpr int ANGULO_PUERTA_ABIERTA = 0;
constexpr int ANGULO_PUERTA_CERRADA = 180;



// ===== VENTANAS DE MEDICION =====
constexpr unsigned long INTERVALO_MUESTREO_ADC_US = 5000UL;
constexpr unsigned long VENTANA_MS = 50;
constexpr unsigned int VENTANAS_POR_PROMEDIO = 20; // ~1 segundo de mediciones

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include "Config.h"
#include "Estado.h"
#include "SensorRuido.h"
#include "Control.h"
#include "Telegram.h"
#include "PanelWeb.h"

// Mantener estos manejadores a nivel global; no deben destruirse al salir de setup().
static WiFiEventHandler eventoDesconexionWiFi;
static WiFiEventHandler eventoIPWiFi;

static void registrarEventosWiFi() {
  eventoDesconexionWiFi = WiFi.onStationModeDisconnected(
    [](const WiFiEventStationModeDisconnected& evento) {
      Serial.printf("\n[WiFi] DESCONECTADO. Motivo: %u\n", evento.reason);
    });

  eventoIPWiFi = WiFi.onStationModeGotIP(
    [](const WiFiEventStationModeGotIP& evento) {
      Serial.print(F("\n[WiFi] IP obtenida: "));
      Serial.println(WiFi.localIP());
    });
}

static void diagnosticarWiFi() {
  Serial.println(F("\n===== DIAGNOSTICO WIFI ====="));
  const bool conectado = WiFi.status() == WL_CONNECTED;
  Serial.print(F("Estado: "));
  Serial.println(conectado ? F("CONECTADO") : F("DESCONECTADO"));
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());
  Serial.print(F("MAC: "));
  Serial.println(WiFi.macAddress());

  if (conectado) {
    Serial.print(F("IP: "));
    Serial.println(WiFi.localIP());
    Serial.print(F("Gateway: "));
    Serial.println(WiFi.gatewayIP());
    Serial.print(F("Mascara: "));
    Serial.println(WiFi.subnetMask());
    Serial.print(F("BSSID: "));
    Serial.println(WiFi.BSSIDstr());
    Serial.print(F("RSSI: "));
    Serial.print(WiFi.RSSI());
    Serial.println(F(" dBm"));
  } else {
    Serial.println(F("IP: (sin direccion)"));
    if (WiFi.getMode() & WIFI_AP) {
      Serial.print(F("Panel AP: http://"));
      Serial.println(WiFi.softAPIP());
    }
  }
  Serial.println(F("============================"));
}

static void iniciarRed() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  // Prueba de estabilidad: deshabilita el ahorro de energia Wi-Fi.
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print(F("Conectando a WiFi"));
  const unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 10000UL) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("Conectado. Panel: http://"));
    Serial.println(WiFi.localIP());
  } else {
    // Conserva STA para recuperar Internet si luego aparece el router.
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    if (WiFi.softAP(AP_NOMBRE, AP_CLAVE)) {
      Serial.print(F("Red de respaldo: "));
      Serial.println(AP_NOMBRE);
      Serial.print(F("Panel: http://"));
      Serial.println(WiFi.softAPIP());
    } else {
      Serial.println(F("Error creando la red AP"));
    }
  }

  // NTP: hora local de Argentina (UTC-3), sin horario de verano.
  // Nunca bloquear esperando sincronizacion: el panel funciona sin Internet.
  if (HABILITAR_TELEGRAM) {
    configTime(-3 * 3600, 0, "pool.ntp.org", "time.google.com");
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(F("\n=== MONITOR DE RUIDO ==="));
  registrarEventosWiFi();
  if (HABILITAR_CONTROL) iniciarControl();
  iniciarRed();
  if (HABILITAR_PANEL_WEB) iniciarPanelWeb();
  Serial.printf("Modulos: panel=%d sensor=%d control=%d telegram=%d\n",
                HABILITAR_PANEL_WEB, HABILITAR_SENSOR,
                HABILITAR_CONTROL, HABILITAR_TELEGRAM);
  if (!SERVO_HABILITADO) {
    Serial.println(F("Servo deshabilitado (sin fuente externa)."));
  }
}

void loop() {
  if (DIAGNOSTICO_WIFI_HABILITADO) {
    static unsigned long ultimoDiagnostico = 0;
    const unsigned long ahora = millis();
    if (ahora - ultimoDiagnostico >= INTERVALO_DIAGNOSTICO_WIFI_MS) {
      ultimoDiagnostico = ahora;
      diagnosticarWiFi();
    }
  }

  // La medicion usa ventanas de 50 ms y entrega un nivel cada 20 ventanas.
  const bool nuevaMedicion = HABILITAR_SENSOR && actualizarSensorRuido();
  if (nuevaMedicion && HABILITAR_CONTROL) actualizarLogicaControl();

  if (HABILITAR_CONTROL) actualizarSalidas();
  if (HABILITAR_PANEL_WEB) atenderPanelWeb();

  if (nuevaMedicion) {
    Serial.printf("Amplitud: %u | dB estimados: %.1f | Estado: %s\n",
                  amplitudActual, nivelDb, nombreEstado());
    if (HABILITAR_TELEGRAM) revisarAvisoFueraDeHorario();
  }
  yield();
}

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <time.h>
#include "Config.h"
#include "Estado.h"
#include "Telegram.h"

static bool eventoActivo = false;
static unsigned long inicioEvento = 0;
static unsigned long inicioPausa = 0;
static unsigned long ultimoAviso = 0;
static bool avisoEnviadoAlgunaVez = false;
static unsigned long ultimaTentativa = 0;
static bool huboTentativa = false;

bool horaSincronizada() {
  return time(nullptr) >= 1735689600; // 01/01/2025 UTC
}

bool fueraDeHorario() {
  if (!horaSincronizada()) return false;
  const time_t ahora = time(nullptr);
  tm horaLocal;
  localtime_r(&ahora, &horaLocal);
  const bool finDeSemana = horaLocal.tm_wday == 0 || horaLocal.tm_wday == 6;
  if (finDeSemana && !parametros.actividadFinDeSemana) return true;
  return horaLocal.tm_hour < static_cast<int>(parametros.horaInicio) ||
         horaLocal.tm_hour >= static_cast<int>(parametros.horaFin);
}

static String codificarURL(const String& texto) {
  static const char hex[] = "0123456789ABCDEF";
  String salida;
  salida.reserve(texto.length() * 3);
  for (unsigned int i = 0; i < texto.length(); ++i) {
    const uint8_t c = static_cast<uint8_t>(texto[i]);
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.') {
      salida += static_cast<char>(c);
    } else {
      salida += '%';
      salida += hex[c >> 4];
      salida += hex[c & 0x0F];
    }
  }
  return salida;
}

bool enviarTelegram(const String& mensaje) {
  // Bloquea tambien las pruebas manuales iniciadas desde el panel.
  if (!HABILITAR_TELEGRAM) {
    Serial.println(F("Telegram deshabilitado desde Config.h"));
    return false;
  }
  if (WiFi.status() != WL_CONNECTED) return false;
  if (String(TELEGRAM_BOT_TOKEN).startsWith("TU_") ||
      String(TELEGRAM_CHAT_ID).startsWith("TU_")) {
    Serial.println(F("Falta configurar token o chat ID"));
    return false;
  }

  // Como describe el informe: prototipo con TLS sin validar certificado.
  // NO desplegar en redes no confiables sin verificar CA del servidor.
  BearSSL::WiFiClientSecure cliente;
  cliente.setInsecure();
  cliente.setTimeout(4000);

  HTTPClient http;
  const String url = String("https://api.telegram.org/bot") + TELEGRAM_BOT_TOKEN + "/sendMessage";
  if (!http.begin(cliente, url)) return false;
  http.setTimeout(4000);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  const String datos = "chat_id=" + codificarURL(TELEGRAM_CHAT_ID) +
                       "&text=" + codificarURL(mensaje);
  const int codigo = http.POST(datos);
  http.end();
  Serial.printf("Telegram HTTP: %d\n", codigo);
  return codigo == HTTP_CODE_OK;
}

void revisarAvisoFueraDeHorario() {
  const unsigned long ahora = millis();
  const float umbralBajo = parametros.umbralAltoDb - parametros.histeresisDb;

  // Un mismo evento permite pausas inferiores a toleranciaMs.
  if (nivelDb >= parametros.umbralAltoDb) {
    if (!eventoActivo) {
      eventoActivo = true;
      inicioEvento = ahora;
    }
    inicioPausa = 0;
  } else if (eventoActivo && nivelDb < umbralBajo) {
    if (inicioPausa == 0) inicioPausa = ahora;
    if (ahora - inicioPausa >= parametros.toleranciaMs) {
      eventoActivo = false;
      inicioPausa = 0;
    }
  }

  if (!eventoActivo || ahora - inicioEvento < parametros.tiempoAvisoMs) return;
  if (!horaSincronizada() || !fueraDeHorario()) return;
  if (WiFi.status() != WL_CONNECTED) return;
  if (avisoEnviadoAlgunaVez && ahora - ultimoAviso < parametros.enfriamientoAvisoMs) return;
  if (huboTentativa && ahora - ultimaTentativa < 30000UL) return;

  huboTentativa = true;
  ultimaTentativa = ahora;

  const time_t tiempo = time(nullptr);
  tm local;
  localtime_r(&tiempo, &local);
  char fecha[28];
  strftime(fecha, sizeof(fecha), "%d/%m/%Y %H:%M:%S", &local);

  String texto = "ALERTA: ruido fuera de horario\n";
  texto += "Nivel estimado: " + String(nivelDb, 1) + " dB (calibrados)\n";
  texto += "Fecha/hora Argentina: " + String(fecha) + "\n";
  texto += "Puerta ordenada: ";
  texto += ordenPuertaCerrada ? "CERRADA" : "ABIERTA";
#if !SERVO_HABILITADO
  texto += " (servo sin conectar)";
#endif
  texto += "\nDuracion: " + String((ahora - inicioEvento) / 1000UL) + " segundos";

  if (enviarTelegram(texto)) {
    ultimoAviso = millis();
    avisoEnviadoAlgunaVez = true;
  }
}

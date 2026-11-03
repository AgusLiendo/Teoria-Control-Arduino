#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "Config.h"
#include "Estado.h"
#include "Telegram.h"
#include "PanelHtml.h"
#include "PanelWeb.h"

static ESP8266WebServer servidor(80);

static String textoBool(bool x) { return x ? "true" : "false"; }

static String fechaActual() {
  if (!horaSincronizada()) return "";
  const time_t ahora = time(nullptr);
  tm local;
  localtime_r(&ahora, &local);
  char fecha[26];
  strftime(fecha, sizeof(fecha), "%d/%m/%Y %H:%M:%S", &local);
  return String(fecha);
}

static void responderEstado() {
  const bool conectado = WiFi.status() == WL_CONNECTED;
  const String ip = conectado ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
  String json;
  json.reserve(1000);
  json += "{\"nivelDb\":" + String(nivelDb, 1);
  json += ",\"amplitud\":" + String(amplitudActual);
  json += ",\"estado\":\"" + String(nombreEstado()) + "\"";
  json += ",\"modo\":\"" + String(nombreModo()) + "\"";
#if SERVO_HABILITADO
  json += ",\"puerta\":\"";
  json += ordenPuertaCerrada ? "CERRANDO/CERRADA\"" : "ABRIENDO/ABIERTA\"";
#else
  json += ",\"puerta\":\"SIN SERVO (orden: ";
  json += ordenPuertaCerrada ? "CERRAR)\"" : "ABRIR)\"";
#endif
  json += ",\"wifi\":" + textoBool(conectado);
  json += ",\"ip\":\"" + ip + "\"";
  json += ",\"horaValida\":" + textoBool(horaSincronizada());
  json += ",\"hora\":\"" + fechaActual() + "\"";
  json += ",\"fueraHorario\":" + textoBool(fueraDeHorario());
  json += ",\"config\":{";
  json += "\"umbral\":" + String(parametros.umbralAltoDb, 1);
  json += ",\"histeresis\":" + String(parametros.histeresisDb, 1);
  json += ",\"offset\":" + String(parametros.offsetDb, 1);
  json += ",\"cierre\":" + String(parametros.tiempoCierreMs / 1000UL);
  json += ",\"reapertura\":" + String(parametros.tiempoReaperturaMs / 1000UL);
  json += ",\"tolerancia\":" + String(parametros.toleranciaMs / 1000UL);
  json += ",\"preaviso\":" + String(parametros.preavisoMs / 1000UL);
  json += ",\"aviso\":" + String(parametros.tiempoAvisoMs / 1000UL);
  json += ",\"enfriamiento\":" + String(parametros.enfriamientoAvisoMs / 1000UL);
  json += ",\"horaInicio\":" + String(parametros.horaInicio);
  json += ",\"horaFin\":" + String(parametros.horaFin);
  json += ",\"finSemana\":" + textoBool(parametros.actividadFinDeSemana);
  json += ",\"silencio\":" + textoBool(parametros.buzzerSilenciado);
  json += ",\"modo\":\"";
  json += parametros.modoPuerta == ModoPuerta::AUTOMATICO ? "AUTO" :
          (parametros.modoPuerta == ModoPuerta::ABIERTA ? "ABIERTA" : "CERRADA");
  json += "\"}}";
  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send(200, "application/json; charset=utf-8", json);
}

static bool autorizar() {
  if (servidor.authenticate(PANEL_USUARIO, PANEL_CLAVE)) return true;
  servidor.requestAuthentication();
  return false;
}

static bool leerNumero(const char* nombre, float minimo, float maximo, float& destino) {
  if (!servidor.hasArg(nombre)) return false;
  String dato = servidor.arg(nombre);
  dato.trim();
  if (dato.length() == 0) return false;
  char* final = nullptr;
  float valor = strtof(dato.c_str(), &final);
  if (final == dato.c_str() || *final != '\0' || !isfinite(valor) ||
      valor < minimo || valor > maximo) return false;
  destino = valor;
  return true;
}

static bool leerEntero(const char* nombre, int minimo, int maximo, int& destino) {
  if (!servidor.hasArg(nombre)) return false;
  String dato = servidor.arg(nombre);
  dato.trim();
  if (dato.length() == 0) return false;
  char* final = nullptr;
  long valor = strtol(dato.c_str(), &final, 10);
  if (final == dato.c_str() || *final != '\0' || valor < minimo || valor > maximo) return false;
  destino = static_cast<int>(valor);
  return true;
}

static void responderConfig() {
  if (!autorizar()) return;
  Parametros nuevo = parametros;
  float valorF;
  int valorI;
#define TOMAR_FLOAT(nombre, minimo, maximo, campo) \
  if (!leerNumero(nombre, minimo, maximo, valorF)) { servidor.send(400, "text/plain", "Valor invalido: " nombre); return; } \
  nuevo.campo = valorF;
#define TOMAR_SEG(nombre, minimo, maximo, campo) \
  if (!leerEntero(nombre, minimo, maximo, valorI)) { servidor.send(400, "text/plain", "Valor invalido: " nombre); return; } \
  nuevo.campo = static_cast<unsigned long>(valorI) * 1000UL;
  TOMAR_FLOAT("umbral", 30, 100, umbralAltoDb)
  TOMAR_FLOAT("histeresis", 1, 15, histeresisDb)
  TOMAR_FLOAT("offset", -30, 70, offsetDb)
  TOMAR_SEG("cierre", 5, 3600, tiempoCierreMs)
  TOMAR_SEG("reapertura", 2, 3600, tiempoReaperturaMs)
  TOMAR_SEG("tolerancia", 1, 60, toleranciaMs)
  TOMAR_SEG("preaviso", 1, 30, preavisoMs)
  TOMAR_SEG("aviso", 5, 1800, tiempoAvisoMs)
  TOMAR_SEG("enfriamiento", 60, 3600, enfriamientoAvisoMs)
#undef TOMAR_FLOAT
#undef TOMAR_SEG
  if (!leerEntero("horaInicio", 0, 23, valorI)) { servidor.send(400,"text/plain","Hora inicio invalida"); return; }
  nuevo.horaInicio = valorI;
  if (!leerEntero("horaFin", 1, 24, valorI)) { servidor.send(400,"text/plain","Hora fin invalida"); return; }
  nuevo.horaFin = valorI;
  if (nuevo.horaInicio >= nuevo.horaFin || nuevo.histeresisDb >= nuevo.umbralAltoDb ||
      nuevo.preavisoMs > nuevo.tiempoCierreMs) {
    servidor.send(400, "text/plain", "Inconsistencia: horario, umbrales o preaviso"); return;
  }
  if (!servidor.hasArg("modo")) { servidor.send(400,"text/plain","Falta modo"); return; }
  const String modo = servidor.arg("modo");
  if (modo == "AUTO") nuevo.modoPuerta = ModoPuerta::AUTOMATICO;
  else if (modo == "ABIERTA") nuevo.modoPuerta = ModoPuerta::ABIERTA;
  else if (modo == "CERRADA") nuevo.modoPuerta = ModoPuerta::CERRADA;
  else { servidor.send(400,"text/plain","Modo invalido"); return; }
  nuevo.actividadFinDeSemana = servidor.arg("finSemana") == "1";
  nuevo.buzzerSilenciado = servidor.arg("silencio") == "1";
  parametros = nuevo;
  servidor.send(200, "text/plain; charset=utf-8", "Valores aplicados (hasta reiniciar la placa)");
}

static void probarTelegram() {
  if (!autorizar()) return;
  String mensaje = "Prueba del monitor de ruido\nNivel: " + String(nivelDb,1) +
                   " dB estimados\nEstado: " + String(nombreEstado()) +
                   "\nHora: " + fechaActual();
  if (enviarTelegram(mensaje)) servidor.send(200, "text/plain; charset=utf-8", "Telegram: mensaje enviado");
  else servidor.send(503, "text/plain; charset=utf-8", "No se pudo enviar a Telegram; revisar WiFi, token y chat ID");
}

void iniciarPanelWeb() {
  servidor.on("/", HTTP_GET, []() {
    servidor.send_P(200, "text/html; charset=utf-8", PAGINA_HTML);
  });
  servidor.on("/api/estado", HTTP_GET, responderEstado);
  servidor.on("/api/config", HTTP_POST, responderConfig);
  servidor.on("/api/prueba-telegram", HTTP_POST, probarTelegram);
  servidor.onNotFound([]() { servidor.send(404, "text/plain", "Ruta no encontrada"); });
  servidor.begin();
  Serial.println(F("Servidor web iniciado, puerto 80"));
}

void atenderPanelWeb() { servidor.handleClient(); }

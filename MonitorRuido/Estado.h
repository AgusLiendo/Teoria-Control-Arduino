#pragma once
#include <Arduino.h>
#include "Config.h"

enum class EstadoRuido : uint8_t { NORMAL, PRECAUCION, CERRADA };
enum class ModoPuerta : uint8_t { AUTOMATICO, ABIERTA, CERRADA };

struct Parametros {
  // Nivel de ruido: valores iniciales definidos en Config.h.
  float umbralAltoDb = UMBRAL_RUIDO_DB;
  float histeresisDb = HISTERESIS_DB;
  float offsetDb = OFFSET_CALIBRACION_DB; // Estimacion; no equivale a dB(A) calibrados.

  // Duraciones: Config.h usa segundos, la logica de control usa milisegundos.
  unsigned long tiempoCierreMs = TIEMPO_CIERRE_S * 1000UL;
  unsigned long tiempoReaperturaMs = TIEMPO_REAPERTURA_S * 1000UL;
  unsigned long toleranciaMs = TOLERANCIA_PAUSAS_S * 1000UL;
  unsigned long preavisoMs = PREAVISO_BUZZER_S * 1000UL;

  // Notificaciones Telegram.
  unsigned long tiempoAvisoMs = TIEMPO_AVISO_TELEGRAM_S * 1000UL;
  unsigned long enfriamientoAvisoMs = ESPERA_ENTRE_ALERTAS_S * 1000UL;

  // Horario de actividad habitual.
  unsigned int horaInicio = HORA_INICIO_HABITUAL;
  unsigned int horaFin = HORA_FIN_HABITUAL;
  bool actividadFinDeSemana = ACTIVIDAD_FIN_SEMANA;

  // Ajuste inicial del buzzer.
  bool buzzerSilenciado = BUZZER_SILENCIADO;

  // El modo de la puerta sigue siendo un control operativo del panel.
  ModoPuerta modoPuerta = ModoPuerta::AUTOMATICO;
};

extern Parametros parametros;
extern EstadoRuido estadoRuido;
extern float nivelDb;
extern unsigned int amplitudActual;
extern bool ordenPuertaCerrada;

const char* nombreEstado();
const char* nombreModo();

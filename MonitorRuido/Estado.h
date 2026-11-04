#pragma once
#include <Arduino.h>
#include "Config.h"

enum class EstadoRuido : uint8_t { NORMAL, PRECAUCION, CERRADA };
enum class ModoPuerta : uint8_t { AUTOMATICO, ABIERTA, CERRADA };

struct Parametros {
  float umbralAltoDb = UMBRAL_RUIDO_DB;
  float histeresisDb = 3.0f;
  float offsetDb = OFFSET_CALIBRACION_DB; // Inicial, REQUIERE calibracion contra sonometro.

  unsigned long tiempoCierreMs = 180000UL;      // 3 minutos
  unsigned long tiempoReaperturaMs = 120000UL;  // 2 minutos
  unsigned long toleranciaMs = 10000UL;        // 10 segundos
  unsigned long preavisoMs = 5000UL;
  unsigned long tiempoAvisoMs = 60000UL;       // Telegram: 60 segundos
  unsigned long enfriamientoAvisoMs = 300000UL; // Telegram: 5 minutos

  unsigned int horaInicio = 7;
  unsigned int horaFin = 22;
  bool actividadFinDeSemana = false;
  bool buzzerSilenciado = false;
  ModoPuerta modoPuerta = ModoPuerta::AUTOMATICO;
};

extern Parametros parametros;
extern EstadoRuido estadoRuido;
extern float nivelDb;
extern unsigned int amplitudActual;
extern bool ordenPuertaCerrada;

const char* nombreEstado();
const char* nombreModo();

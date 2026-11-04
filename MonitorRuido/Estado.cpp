#include "Estado.h"

Parametros parametros;
EstadoRuido estadoRuido = EstadoRuido::NORMAL;
float nivelDb = 0.0f;
unsigned int amplitudActual = 0;
bool ordenPuertaCerrada = false;

const char* nombreEstado() {
  switch (estadoRuido) {
    case EstadoRuido::NORMAL: return "NORMAL";
    case EstadoRuido::PRECAUCION: return "PRECAUCION";
    case EstadoRuido::CERRADA: return "CERRADA";
  }
  return "DESCONOCIDO";
}

const char* nombreModo() {
  switch (parametros.modoPuerta) {
    case ModoPuerta::AUTOMATICO: return "AUTOMATICO";
    case ModoPuerta::ABIERTA: return "ABIERTA";
    case ModoPuerta::CERRADA: return "CERRADA";
  }
  return "DESCONOCIDO";
}

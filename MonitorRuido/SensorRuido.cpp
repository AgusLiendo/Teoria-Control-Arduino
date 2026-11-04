#include <Arduino.h>
#include <math.h>
#include "Config.h"
#include "Estado.h"
#include "SensorRuido.h"

// El ESP8266 comparte recursos entre su ADC y la radio WiFi.
// No se debe llamar analogRead() de forma continua en un while.
// Este muestreador toma UNA muestra por llamada, y solo si
// pasaron al menos INTERVALO_MUESTREO_ADC_US microsegundos.

static unsigned long ultimaMuestraUs = 0;
static unsigned long inicioVentanaMs = 0;
static bool muestreoInicializado = false;
static int minimoVentana = 1023;
static int maximoVentana = 0;
static unsigned int muestrasVentana = 0;
static double sumaAmplitudesCuadradas = 0.0;
static unsigned int ventanasAcumuladas = 0;

bool actualizarSensorRuido() {
  const unsigned long ahoraUs = micros();
  if (muestreoInicializado &&
      static_cast<unsigned long>(ahoraUs - ultimaMuestraUs) < INTERVALO_MUESTREO_ADC_US) {
    return false;
  }

  muestreoInicializado = true;
  ultimaMuestraUs = ahoraUs;

  const int lectura = analogRead(PIN_MICROFONO);
  const unsigned long ahoraMs = millis();

  if (muestrasVentana == 0) {
    inicioVentanaMs = ahoraMs;
    minimoVentana = lectura;
    maximoVentana = lectura;
  } else {
    if (lectura < minimoVentana) minimoVentana = lectura;
    if (lectura > maximoVentana) maximoVentana = lectura;
  }
  ++muestrasVentana;

  // Una ventana dura aproximadamente 50 ms, sin bloquear el loop.
  if (static_cast<unsigned long>(ahoraMs - inicioVentanaMs) < VENTANA_MS) {
    return false;
  }

  amplitudActual = static_cast<unsigned int>(maximoVentana - minimoVentana);
  const double amplitud = static_cast<double>(amplitudActual);
  sumaAmplitudesCuadradas += amplitud * amplitud;
  ++ventanasAcumuladas;
  muestrasVentana = 0;

  if (ventanasAcumuladas < VENTANAS_POR_PROMEDIO) {
    return false;
  }

  const double promedioCuadratico = sumaAmplitudesCuadradas / ventanasAcumuladas;
  const double amplitudEfectiva = sqrt(promedioCuadratico);
  const float amplitudSegura = amplitudEfectiva > 1.0
      ? static_cast<float>(amplitudEfectiva) : 1.0f;
  nivelDb = 20.0f * log10f(amplitudSegura) + parametros.offsetDb;

  sumaAmplitudesCuadradas = 0.0;
  ventanasAcumuladas = 0;
  return true;
}

#include <Arduino.h>
#include "Config.h"
#include "Estado.h"
#include "Control.h"

#if SERVO_HABILITADO
#include <Servo.h>
static Servo servo;
static int anguloActual = ANGULO_PUERTA_ABIERTA;
static unsigned long ultimoPasoServo = 0;
#endif

static unsigned long inicioPrecaucion = 0;
static unsigned long inicioCalma = 0;

static void cambiarEstado(EstadoRuido nuevo) {
  estadoRuido = nuevo;
  inicioCalma = 0;
  if (nuevo == EstadoRuido::PRECAUCION) inicioPrecaucion = millis();
  Serial.print(F("Nuevo estado: "));
  Serial.println(nombreEstado());
}

void iniciarControl() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
#if SERVO_HABILITADO
  servo.attach(PIN_SERVO);
  servo.write(ANGULO_PUERTA_ABIERTA);
#endif
}

void actualizarLogicaControl() {
  const unsigned long ahora = millis();
  const float umbralBajo = parametros.umbralAltoDb - parametros.histeresisDb;

  switch (estadoRuido) {
    case EstadoRuido::NORMAL:
      if (nivelDb >= parametros.umbralAltoDb) {
        cambiarEstado(EstadoRuido::PRECAUCION);
      }
      break;

    case EstadoRuido::PRECAUCION:
      if (nivelDb < umbralBajo) {
        if (inicioCalma == 0) inicioCalma = ahora;
        if (ahora - inicioCalma >= parametros.toleranciaMs) {
          cambiarEstado(EstadoRuido::NORMAL);
          break;
        }
      } else {
        inicioCalma = 0;
      }
      if (ahora - inicioPrecaucion >= parametros.tiempoCierreMs) {
        cambiarEstado(EstadoRuido::CERRADA);
      }
      break;

    case EstadoRuido::CERRADA:
      if (nivelDb < umbralBajo) {
        if (inicioCalma == 0) inicioCalma = ahora;
        if (ahora - inicioCalma >= parametros.tiempoReaperturaMs) {
          cambiarEstado(EstadoRuido::NORMAL);
        }
      } else {
        inicioCalma = 0;
      }
      break;
  }
}

void actualizarSalidas() {
  const unsigned long ahora = millis();
  const bool enPreaviso = estadoRuido == EstadoRuido::PRECAUCION &&
       ahora - inicioPrecaucion < parametros.tiempoCierreMs &&
       parametros.tiempoCierreMs - (ahora - inicioPrecaucion) <= parametros.preavisoMs;
  const bool sonar = !parametros.buzzerSilenciado && enPreaviso && (ahora / 300) % 2 == 0;
  digitalWrite(PIN_BUZZER, sonar ? HIGH : LOW);

  if (parametros.modoPuerta == ModoPuerta::ABIERTA) ordenPuertaCerrada = false;
  else if (parametros.modoPuerta == ModoPuerta::CERRADA) ordenPuertaCerrada = true;
  else ordenPuertaCerrada = estadoRuido == EstadoRuido::CERRADA;

#if SERVO_HABILITADO
  if (ahora - ultimoPasoServo >= 20) {
    ultimoPasoServo = ahora;
    const int destino = ordenPuertaCerrada ? ANGULO_PUERTA_CERRADA : ANGULO_PUERTA_ABIERTA;
    if (anguloActual < destino) anguloActual = min(anguloActual + 2, destino);
    if (anguloActual > destino) anguloActual = max(anguloActual - 2, destino);
    servo.write(anguloActual);
  }
#endif
}

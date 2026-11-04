#pragma once
#include <Arduino.h>

bool horaSincronizada();
bool fueraDeHorario();
bool enviarTelegram(const String& mensaje);
void revisarAvisoFueraDeHorario(); // llamar con cada promedio nuevo

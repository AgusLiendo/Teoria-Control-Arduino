#pragma once

void iniciarControl();
void actualizarLogicaControl(); // Llamar cuando haya una nueva medicion (1 s)
void actualizarSalidas();       // Llamar frecuentemente (no bloqueante)

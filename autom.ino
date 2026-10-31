
const int PIN_MIC = A0;
const int PIN_BUZZER = D6;

// CONFIGURACIÓN
const int UMBRAL = 80;      // Sensibilidad al sonido
const int VENTANA_MS = 50;   // Tiempo de medición
const int DURACION_PITIDO = 200;

// Medir intensidad del sonido
int medirNivel() {
  int minimo = 1023;
  int maximo = 0;

  unsigned long inicio = millis();

  while (millis() - inicio < VENTANA_MS) {
    int valor = analogRead(PIN_MIC);

    if (valor < minimo) minimo = valor;
    if (valor > maximo) maximo = valor;
  }

  return maximo - minimo;
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  Serial.println("Sistema iniciado");
}

void loop() {
  int nivel = medirNivel();

  Serial.print("Nivel de sonido: ");
  Serial.println(nivel);

  if (nivel >= UMBRAL) {
    Serial.println("UMBRAL SUPERADO!");

    // Encender buzzer
    digitalWrite(PIN_BUZZER, HIGH);

    delay(DURACION_PITIDO);

    // Apagar buzzer
    digitalWrite(PIN_BUZZER, LOW);

    delay(300);
  }
}

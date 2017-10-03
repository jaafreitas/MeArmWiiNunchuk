/*

  Intruções:
  * Joystick X movimenta a base.
  * Joystick Y movimenta a distância.
  * Acelerômetro Y movimenta a altura.
  * botão Z fecha a garra enquanto apertado.

  Pendências:
  * botão C trava o braço quando apertado uma vez, destrava quando apertado novamente.
  * modo demo fazendo alguma coisa.
  * utilizar conceito de coordenadas? https://github.com/yorkhackspace/meArm

  Observações:
  * O Nunchuk parece funcionar mais estável quando alimentado em 3.3V no Arduino Nano.

  Informações técnicas:
  * http://wiibrew.org/wiki/Wiimote/Extension_Controllers/Nunchuck

 */
#include <Wire.h>
#include <Servo.h>
#include "WiiNunchuk.h"

#define SERVO_BASE_PIN 3
#define SERVO_DISTANCIA_PIN 6
#define SERVO_ALTURA_PIN 9
#define SERVO_GARRA_PIN 12

#define NUNCHUK_JOY_X_MIN 20
#define NUNCHUK_JOY_X_CENTRO 127
#define NUNCHUK_JOY_X_MAX 220

#define NUNCHUK_JOY_Y_MIN 35
#define NUNCHUK_JOY_Y_CENTRO 133
#define NUNCHUK_JOY_Y_MAX 228

// AccY varia aproximadamente entre 280 e 720.
#define NUNCHUK_ACC_Y_MIN 350
#define NUNCHUK_ACC_Y_CENTRO 512
#define NUNCHUK_ACC_Y_MAX 640


// Quando conectado na USB, precisamos de pelo menos 30 milisegundos para
// garantirmos a estabilidade na leitura.
#define SERVO_MIN_ATUALIZACAO 30

#define AMOSTRAS SERVO_MIN_ATUALIZACAO

unsigned long joyxTotal = 0;
uint8_t joyx[AMOSTRAS];

unsigned long joyyTotal = 0;
uint8_t joyy[AMOSTRAS];

unsigned long accyTotal = 0;
uint16_t accy[AMOSTRAS];

unsigned int indiceAmostra = 0;

Servo servoBase, servoDistancia, servoAltura, servoGarra;

unsigned long tempoAnterior = 0;

void setup() {  
  Serial.begin(115200);
  Serial.println();
  Serial.println("*** MeArm ***");

  for (int i = 0; i < AMOSTRAS; i++) {
    joyx[i] = NUNCHUK_JOY_X_CENTRO;
    joyxTotal += joyx[i];
    
    joyy[i] = NUNCHUK_JOY_Y_CENTRO;
    joyyTotal += joyy[i];
    
    accy[i] = NUNCHUK_ACC_Y_CENTRO;
    accyTotal += accy[i];
  }

  nunchuk_init();

  servoBase.attach(SERVO_BASE_PIN);
  servoDistancia.attach(SERVO_DISTANCIA_PIN);
  servoAltura.attach(SERVO_ALTURA_PIN);
  servoGarra.attach(SERVO_GARRA_PIN);

  // Posição de descanso caso o Nunchuk não esteja conectado.
  servoBase.write(80);
  servoDistancia.write(60);
  servoAltura.write(60);
  servoGarra.write(5);
}

void loop() {
  if (nunchuk_get_data()) {
    joyxTotal = joyxTotal - joyx[indiceAmostra];
    joyx[indiceAmostra] = nunchuk_joyx();
    joyxTotal = joyxTotal + joyx[indiceAmostra];
    unsigned long joyxAverage = joyxTotal / AMOSTRAS;
    int servoBaseAngulo;
    if (joyxAverage >= NUNCHUK_JOY_X_CENTRO) {
      servoBaseAngulo = max(map(joyxAverage, NUNCHUK_JOY_X_CENTRO, NUNCHUK_JOY_X_MAX, 80, 10), 10);
    } else {
      servoBaseAngulo = min(map(joyxAverage, NUNCHUK_JOY_X_MIN, NUNCHUK_JOY_X_CENTRO, 165, 80), 165);
    }

    joyyTotal = joyyTotal - joyy[indiceAmostra];
    joyy[indiceAmostra] = nunchuk_joyy();
    joyyTotal = joyyTotal + joyy[indiceAmostra];
    unsigned long joyyAverage = joyyTotal / AMOSTRAS;
    int servoDistanciaAngulo;
    if (joyyAverage >= NUNCHUK_JOY_Y_CENTRO) {
      servoDistanciaAngulo = min(map(joyyAverage, NUNCHUK_JOY_Y_CENTRO, NUNCHUK_JOY_Y_MAX, 60, 150), 150);
    } else {
      servoDistanciaAngulo = max(map(joyyAverage, NUNCHUK_JOY_Y_MIN, NUNCHUK_JOY_Y_CENTRO, 20, 60), 20);
    }

    accyTotal = accyTotal - accy[indiceAmostra];
    accy[indiceAmostra] = nunchuk_accely();
    accyTotal = accyTotal + accy[indiceAmostra];
    unsigned long accyAverage = accyTotal / AMOSTRAS;
    int servoAlturaAngulo;
    if (accyAverage >= NUNCHUK_ACC_Y_CENTRO) {
      servoAlturaAngulo = max(map(accyAverage, NUNCHUK_ACC_Y_CENTRO, NUNCHUK_ACC_Y_MAX, 90, 60), 60);
    } else {
      servoAlturaAngulo = min(map(accyAverage, NUNCHUK_ACC_Y_MIN, NUNCHUK_ACC_Y_CENTRO, 165, 90), 165);
    }

    indiceAmostra += 1;
    if (indiceAmostra >= AMOSTRAS) {
      indiceAmostra = 0;
    }

    bool zbut = nunchuk_zbutton();
    int servoGarraAngulo;
    if (zbut) {
      servoGarraAngulo = 80;
    } else {
      servoGarraAngulo = 5;
    }

    unsigned long tempoAtual = millis();
    if (tempoAtual - tempoAnterior >= SERVO_MIN_ATUALIZACAO) {
      servoBase.write(servoBaseAngulo);
      servoDistancia.write(servoDistanciaAngulo);
      servoAltura.write(servoAlturaAngulo);
      servoGarra.write(servoGarraAngulo);

      tempoAnterior = tempoAtual;
    }

    // Informações para depuração.
    Serial.print(tempoAtual); Serial.print(" ms");

    Serial.print(" | Joy.X / Base: "); Serial.print(joyxAverage);
    Serial.print(" / "); Serial.print(servoBaseAngulo); Serial.print("°");

    Serial.print(" | Joy.Y / Distância: "); Serial.print(joyyAverage);
    Serial.print(" / "); Serial.print(servoDistanciaAngulo); Serial.print("°");

    Serial.print(" | Ac.Y / Altura: "); Serial.print(accyAverage);
    Serial.print(" / "); Serial.print(servoAlturaAngulo); Serial.print("°");

    Serial.print(" | Bot.Z: "); Serial.print(zbut);

    Serial.println();
  }
}

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

#define SERVO_BASE_PIN 4
#define SERVO_DISTANCIA_PIN 5
#define SERVO_ALTURA_PIN 6
#define SERVO_GARRA_PIN 7

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
    accy[i] = 512;
    accyTotal += accy[i];
  }

  nunchuk_init();

  servoBase.attach(SERVO_BASE_PIN);
  servoDistancia.attach(SERVO_DISTANCIA_PIN);
  servoAltura.attach(SERVO_ALTURA_PIN);
  servoGarra.attach(SERVO_GARRA_PIN);

  servoBase.write(90);
  servoDistancia.write(90);
  servoAltura.write(135);
  servoGarra.write(90);
}

void loop() {
  if (nunchuk_get_data()) {
    uint8_t joyx = nunchuk_joyx();
    int servoBaseAngulo;
    if (joyx >= NUNCHUK_JOY_X_CENTRO) {
      servoBaseAngulo = max(map(joyx, NUNCHUK_JOY_X_CENTRO, NUNCHUK_JOY_X_MAX, 90, 0), 0);
    } else {
      servoBaseAngulo = min(map(joyx, NUNCHUK_JOY_X_MIN, NUNCHUK_JOY_X_CENTRO, 180, 90), 180);
    }

    uint8_t joyy = nunchuk_joyy();
    int servoDistanciaAngulo;
    if (joyy >= NUNCHUK_JOY_Y_CENTRO) {
      servoDistanciaAngulo = min(map(joyy, NUNCHUK_JOY_Y_CENTRO, NUNCHUK_JOY_Y_MAX, 90, 180), 180);
    } else {
      servoDistanciaAngulo = max(map(joyy, NUNCHUK_JOY_Y_MIN, NUNCHUK_JOY_Y_CENTRO, 0, 90), 00);
    }

    accyTotal = accyTotal - accy[indiceAmostra];
    accy[indiceAmostra] = nunchuk_accely();
    accyTotal = accyTotal + accy[indiceAmostra];
    unsigned long accyAverage = accyTotal / AMOSTRAS;
    int servoAlturaAngulo;
    if (accyAverage >= NUNCHUK_ACC_Y_CENTRO) {
      servoAlturaAngulo = min(map(accyAverage, NUNCHUK_ACC_Y_CENTRO, NUNCHUK_ACC_Y_MAX, 90, 180), 180);
    } else {
      servoAlturaAngulo = max(map(accyAverage, NUNCHUK_ACC_Y_MIN, NUNCHUK_ACC_Y_CENTRO, 0, 90), 00);
    }

    indiceAmostra += 1;
    if (indiceAmostra >= AMOSTRAS) {
      indiceAmostra = 0;
    }

    bool zbut = nunchuk_zbutton();
    int servoGarraAngulo;
    if (zbut) {
      servoGarraAngulo = 90;
    } else {
      servoGarraAngulo = 0;
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

    Serial.print(" | Joy.X / Base: "); Serial.print(joyx);
    Serial.print(" / "); Serial.print(servoBaseAngulo); Serial.print("°");

    Serial.print(" | Joy.Y / Distância: "); Serial.print(joyy);
    Serial.print(" / "); Serial.print(servoDistanciaAngulo); Serial.print("°");

    Serial.print(" | Ac.Y / Altura: "); Serial.print(accyAverage);
    Serial.print(" / "); Serial.print(servoAlturaAngulo); Serial.print("°");

    Serial.print(" | Bot.Z: "); Serial.print(zbut);

    Serial.println();
  }
}

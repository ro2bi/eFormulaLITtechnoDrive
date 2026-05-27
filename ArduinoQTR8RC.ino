#include <QTRSensors.h>

QTRSensors qtr;

const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

// ===== НАСТРОЙКИ =====
#define BASE_SPEED 190
#define MAX_SPEED 255
#define STRAIGHT_SPEED 255
#define TURN_SPEED 170

// Более плавный PID
const float KP = 0.07;
const float KD = 1.3;

int lastError = 0;

// ===== КНОПКА =====
const int button = 2;
const int led_on = 13;

bool started = false;
unsigned long startTime = 0; // Переменная для хранения времени старта

// ===== TB6612FNG =====
const int AIN1 = 6;
const int AIN2 = 7;
const int PWMA = 11;

const int BIN1 = 4;
const int BIN2 = 3;
const int PWMB = 8;

const int STBY = 12;

// =========================

bool IsStart() {
  return digitalRead(button) == LOW;
}

// =========================

void setMotors(int leftSpeed, int rightSpeed) {

  // Ограничение
  if (leftSpeed > MAX_SPEED) leftSpeed = MAX_SPEED;
  if (rightSpeed > MAX_SPEED) rightSpeed = MAX_SPEED;

  if (leftSpeed < 0) leftSpeed = 0;
  if (rightSpeed < 0) rightSpeed = 0;

  // ВСЕГДА ВПЕРЁД

  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  analogWrite(PWMA, leftSpeed);
  analogWrite(PWMB, rightSpeed);
}


void followLine() {

  int position = qtr.readLineBlack(sensorValues);

  // Центр линии
  int error = position - 3500;

  // PID
  int motorSpeed =
      KP * error +
      KD * (error - lastError);

  lastError = error;

  //СКОРОСТЬ

  int baseSpeed;

  // Прямая
  if (abs(error) < 250) {
    baseSpeed = STRAIGHT_SPEED;
  }
  // Небольшой поворот
  else if (abs(error) < 1200) {
    baseSpeed = 210;
  }
  // Резкий поворот
  else {
    baseSpeed = TURN_SPEED;
  }

  // ================================

  int leftMotorSpeed = baseSpeed + motorSpeed;
  int rightMotorSpeed = baseSpeed - motorSpeed;

  setMotors(leftMotorSpeed, rightMotorSpeed);
}

// =========================

void setup() {

  Serial.begin(9600);

  pinMode(button, INPUT_PULLUP);
  pinMode(led_on, OUTPUT);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);

  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  pinMode(STBY, OUTPUT);

  digitalWrite(STBY, HIGH);

  // QTR
  qtr.setTypeRC();

  qtr.setSensorPins(
    (const uint8_t[]) {
      A0, A1, A2, A3,
      A4, A5, 9, 10
    },
    SensorCount
  );

  // Калибровка
  digitalWrite(led_on, HIGH);

  delay(500);

  for (uint16_t i = 0; i < 250; i++) {
    qtr.calibrate();
    delay(5);
  }

  digitalWrite(led_on, LOW);

  // Стоп
  setMotors(0, 0);
}

// =========================

void loop() {

  if (!started) {

    if (IsStart()) {

      delay(500);

      started = true;
      startTime = millis(); // Запоминаем время фактического старта
    }

    setMotors(0, 0);

    return;
  }

  // Проверяем, прошло ли 20 секунд (20000 миллисекунд)
  if (millis() - startTime >= 20000) {
    started = false;   // Сбрасываем флаг, чтобы робот перешел в режим ожидания кнопки
    setMotors(0, 0);    // Останавливаем моторы
    return;
  }

  followLine();
}
#include <QTRSensors.h>

QTRSensors qtr;
const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

#define MAINRAPID 210  

const int button = 2;   
const int led_on = 13;  

bool IsStarted = false;
bool IsStartDelayed = false;
// фиксация времени в момент нажатия на кнопку
unsigned long p1Time = 0;
// подсчет времени гонки
unsigned long p2Time = 0;

//текущее смещение робота
int place = 0;

//TB6612FNG
const int AIN1 = 6; 
const int AIN2 = 7; 
const int PWMA = 11;
const int BIN1 = 4; 
const int BIN2 = 3; 
const int PWMB = 8; 
const int STBY = 12; 

int lastError = 0;

//проверка нажатия кнопки
bool IsStart() {
  int m_button1 = 0;
  int m_button2 = 0;

  m_button1 = digitalRead(button);
  delay(30);
  m_button2 = digitalRead(button);
  m_button2 = m_button2 | m_button1;
 
  if (m_button2) {
    return false;
  } else {
    return true;
  }
}

//задержка перед стартом
bool StartDelay(bool m_delay) {
  if (!m_delay) {
    delay(2000); 
    return true;     
  }
  return true;
}

//текущая позиция робота отн черной линии
int Placement() {
  int position = qtr.readLineBlack(sensorValues);
  position -= 3500; 
  return position;
}

//агрессивность подруливания
const float KP = 0.08;  
//гашение виляний 
const float KD = 1.35;   

//управление моторами
void SetRapid(int pos, int base_speed) {
    //(Ошибка * KP) + (Разница ошибок * KD)
    int motorSpeed = KP * pos + KD * (pos - lastError);
    lastError = pos;

    int abs_pos = abs(pos);

    //робот едет по прямой
    if (abs_pos < 300) {
        base_speed = 255; 
    }
    //поворот
    else if (abs_pos > 1500) {
        base_speed = base_speed * 0.75; 
    }

    int lmSpeed = base_speed + motorSpeed;
    int rmSpeed = base_speed - motorSpeed;

    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);

    if (lmSpeed < 0) lmSpeed = 0;
    if (rmSpeed < 0) rmSpeed = 0;

    if (lmSpeed > 255) lmSpeed = 255;
    if (rmSpeed > 255) rmSpeed = 255;

    //подача мощности на моторы
    analogWrite(PWMA, lmSpeed);
    analogWrite(PWMB, rmSpeed);
}

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

  //вівод из режима ожидания драйвера моторов
  digitalWrite(STBY, HIGH); 

  //настройка датчиков
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4, A5, 9, 10}, SensorCount);

  //калибровка датчиков
  delay(200);
  digitalWrite(led_on, HIGH); //калибровка
  delay(300);
  for (uint16_t i = 0; i < 200; i++) {
    qtr.calibrate();
  }
  digitalWrite(led_on, LOW); //калибровка окончена
  
  IsStarted = false;
}

void loop() {
  if (IsStarted == false) {
    IsStarted = IsStart();
    p1Time = millis();
  }

  if (IsStarted == true) {
    digitalWrite(led_on, HIGH);
    IsStartDelayed = StartDelay(IsStartDelayed);
  
    place = Placement();
    SetRapid(place, MAINRAPID);
    
    p2Time = millis();
    p2Time -= p1Time;
    
    //стоп через 40 секунд
    if (p2Time > 40000) IsStarted = false;
  } 
  
}
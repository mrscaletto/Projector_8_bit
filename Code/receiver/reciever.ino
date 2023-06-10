#define LED_PIN 13
#define LED_NUM 64
#include "FastLED.h"
CRGB leds[LED_NUM];

class LEDSquareMatrix {
public:
  void init(CRGB* ledsPointer, int size) {
    FastLED.addLeds<WS2812, LED_PIN, GRB>(ledsPointer, size * size);
    FastLED.setBrightness(50);
    this->ledsPointer;
    this->size = size;
    this->fullSize = size * size;
    this->previousTime = millis();
    this->mode = 0;
  }

  int size;
  int fullSize;
  unsigned long previousTime;
  CRGB* ledsPointer;
  int mode;

  void rainbow() {
    static byte counter = 0;
    for (int i = 0; i < fullSize; i++) {
      ledsPointer[i].setHue(counter + i * 255 / fullSize);
    }
    counter++;  // counter меняется от 0 до 255 (тип данных byte)
  }

  void noise() {
    FastLED.clear();
    ledsPointer[random(fullSize)].setHue(random(256));
  }

  void runningLED() {
    static int tar, cur;      // храним позицию и цель
    static byte color;        // храним цвет
    if (tar == cur) {             // позиция совпала
      tar = random(0, fullSize);   // новая случайная позиция
      delay(500);                 // ждём
    }
    cur += (cur < tar) ? 1 : -1;  // направление движения
    // снижаем яркость всей ленты
    for (int i = 0; i < fullSize; i++) leds[i].fadeLightBy(180);
    leds[cur].setHue(color);  // зажигаем светодиод
    color++;                  // меняем цвет
  }

  void increaseMode(int n) {
    mode = (mode + n) % 3;
    FastLED.clear();
  }  

  void update(int deltaT) {
    while ((millis() - previousTime) > deltaT)
      ;
    switch ( mode ) {
      case 0:
        rainbow();
        break;
      case 1:
        noise();
        break;
      case 2:
        runningLED();
        break;
    }
    FastLED.show();           // выводим
    previousTime = millis();
  }

} ledMatrix;

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

#define SERVO_0      8
#define SERVO_1      9
#define SERVO_2      10

class ServoMod : public Servo {
  public:
    void init(int pin, int left, int right) {
      this->attach(pin);
      this->left = left;
      this->right = right;
      this->setPosition((left + right) / 2);
    }
    int left;
    int right;
    int position;
    int getPosition() {
    }
    bool setPosition(int position) {
      if (position >= left && position <= right) {
        this->position = position;
        this->write(this->position);
        return true;
      }
      return false;
    }
    bool incPosition(int delta) {
      setPosition(position + delta);
    }
};

ServoMod servo_0;
ServoMod servo_1;
ServoMod servo_2;

RF24 radio(6, 7); // nRF24L01+ (CE, CSN)
int data[2];

int previousMode;
int previousDirection;

void setup() {
  previousMode = -1;
  previousDirection = -666;
  Serial.begin(9600);

  servo_0.init(SERVO_0, 0, 180);
  servo_1.init(SERVO_1, 0, 180);
  servo_2.init(SERVO_2, 0, 180);
  ledMatrix.init(leds, 8);

  radio.begin();
  radio.setChannel(11);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openReadingPipe(0, 0x1234567890LL);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(data));

    if (previousMode != data[0]) {
      previousMode = data[0];
    }
    else {
      if (previousDirection == 0) {
        switch (previousMode) {
          case 0:
            servo_0.incPosition(data[1]);
            break;
          case 1:
            servo_1.incPosition(data[1]);
            break;
          case 2:
            servo_2.incPosition(data[1]);
            break;
          case 3:
            ledMatrix.increaseMode(data[1]);
            break;
        }
      }
    }
    
    previousDirection = data[1];
    ledMatrix.update(30);


    // Serial.print(servo_1.getPosition());
    // Serial.print(' ');
    // Serial.print(data[1]);
    // Serial.print(' ');
    // Serial.println(data[2]);
  }
}


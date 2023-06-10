Mo
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
      tar = random(0, LED_NUM);   // новая случайная позиция
      delay(500);                 // ждём
    }
    cur += (cur < tar) ? 1 : -1;  // направление движения
    // снижаем яркость всей ленты
    for (int i = 0; i < LED_NUM; i++) leds[i].fadeLightBy(180);
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

};

void
setup() {
}

byte counter;
void loop() {
  for (int i = 0; i < LED_NUM; i++) {
    leds[i].setHue(counter + i * 255 / LED_NUM);
  }
  counter++;  // counter меняется от 0 до 255 (тип данных byte)
  FastLED.show();
  delay(30);  // скорость движения радуги
}
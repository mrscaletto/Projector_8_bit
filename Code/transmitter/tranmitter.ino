#include <SPI.h>                                         
#include <nRF24L01.h>                                    
#include <RF24.h>        

// motors' pins

//#define SPEED_1      5 
//#define DIR_1        4
// 
//#define SPEED_2      6
//#define DIR_2        7

// joystick's pins
#define X_VAL A5
//#define Y_VAL A4
#define Z_VAL A4

#define LED_0 2
#define LED_1 3
#define LED_2 4
#define LED_3 5

#define NUMBER_OF_MODES 4
RF24 radio(8, 9); // nRF24L01+ (CE, CSN)
int data[2]; 
int recieveChannel = 0;

class Button {
  public:
    void init(int pin, int delayTime) {
      this->pin = pin;
      this->delayTime = delayTime;
      this->previousValue = LOW;
    }
    int pin;
    int delayTime;
    int previousValue;
    bool update() {
      int currentValue = digitalRead(this->pin);
      if (currentValue != this->previousValue) {
        delay(10);
        currentValue = digitalRead(this->pin);
      }
      this->previousValue = currentValue;
    }
} button;

void setup() {
  pinMode(LED_0, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);

  pinMode(X_VAL, INPUT);
  pinMode(Z_VAL, INPUT);

  button.init(Z_VAL, 10);

//  Serial.begin(9600);
  //radio configuration
  radio.begin();
  radio.setChannel(11);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openWritingPipe(0x1234567890LL);
  radio.stopListening();
}

void toZero(int* array, int n)
{
  for (int i = 0; i < n; i++)
    array[i] = 511;
}

void ledSwitcher()
{
  digitalWrite(LED_0, LOW);
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, LOW);
  switch (recieveChannel)
  {
    case 0:
      digitalWrite(LED_0, HIGH);
      break;
    case 1:
      digitalWrite(LED_1, HIGH);
      break;
    case 2:
      digitalWrite(LED_2, HIGH);
      break;
    case 3:
      digitalWrite(LED_3, HIGH);
      break;
  }
}

void switcher()
{
  if (button.update() == HIGH) 
  {
    if (recieveChannel == (NUMBER_OF_MODES - 1))
      recieveChannel = 0;
    else
      recieveChannel++;
  }
  ledSwitcher();
}

#define TOP 700
#define BOTTOM 300
int makeDirection(int analogVal) {
  if (analogVal > TOP)
    return 1;
  if (analogVal < BOTTOM)
    return -1;
  return 0;
}

void loop() {
  toZero(data, sizeof(data));
  switcher();
  data[0] = recieveChannel;
  data[1] = makeDirection(analogRead(X_VAL));

  radio.write(&data, sizeof(data));
}

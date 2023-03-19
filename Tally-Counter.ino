#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
// static const unsigned char PROGMEM logo_bmp[] =
// { 0b00000000, 0b11000000,
//   0b00000001, 0b11000000,
//   0b00000001, 0b11000000,
//   0b00000011, 0b11100000,
//   0b11110011, 0b11100000,
//   0b11111110, 0b11111000,
//   0b01111110, 0b11111111,
//   0b00110011, 0b10011111,
//   0b00011111, 0b11111100,
//   0b00001101, 0b01110000,
//   0b00011011, 0b10100000,
//   0b00111111, 0b11100000,
//   0b00111111, 0b11110000,
//   0b01111100, 0b11110000,
//   0b01110000, 0b01110000,
//   0b00000000, 0b00110000 };

class BtnClass{
  public:
    const int debounceThres = 50;
    bool lastSteadyState = LOW;
    bool lastBounceState = LOW;
    int buttonPin;
    bool isPressed;
    bool currentState;
    int* counter;
    int modifier = 0;

    unsigned long int lastDebounceTime;

    BtnClass(int pin, int number, int* count)
    {
      buttonPin = pin;
      modifier = number;
      counter = count;
    }

    void addCounter()
    {
      *counter += modifier;
      if(*counter < 0)
        *counter = 0;
    }

    void buttonCheck()
    {
      currentState = digitalRead(buttonPin);

      if(millis() - lastDebounceTime > debounceThres)
      {
        if(lastSteadyState == HIGH && currentState == LOW)
        {
          isPressed = true;
          addCounter();
        }
        else if(lastSteadyState == LOW && currentState == HIGH)
        {
          isPressed = false;
        }
        
        lastSteadyState = currentState;
      }

      if (currentState != lastBounceState) 
      {
        lastDebounceTime = millis();
        lastBounceState = currentState;
      }
    }  
};

void displayCenter(String text, int X, int Y) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  display.setCursor((SCREEN_WIDTH - width) / 2 + X, (SCREEN_HEIGHT - height) / 2 + Y);
  display.println(text);
}

void tallyCounter(int counter)
{
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(22, 0);
  displayCenter("Counter", 0, -24);

  display.setTextSize(4);
  displayCenter(String(counter), 0, 8);
  display.display();
}

void tallyReset()
{
  display.clearDisplay();
  display.setTextSize(4);
  displayCenter("RESET", 0, 0);
  display.display();
  delay(2000);
}

const int pinUpBtn = 14;
const int pinDownBtn = 12;

int counter = 1000;
unsigned long int resetTime = 0;
bool secondLoop = 0;

BtnClass upButton(pinUpBtn, 1, &counter);
BtnClass downButton(pinDownBtn, -1, &counter);

void setup() {
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.display();
  delay(1000);
  display.clearDisplay();

  pinMode(pinUpBtn, INPUT_PULLUP);
  pinMode(pinDownBtn, INPUT_PULLUP);
}

void loop() {
  tallyCounter(counter);
  upButton.buttonCheck();
  downButton.buttonCheck();

  // for debugging only
  // Serial.printf("UpButtonState:%d,DownButtonState:%d,UpIsPressed:%d,DownIsPressed:%d\n", upButton.currentState, downButton.currentState, upButton.isPressed, downButton.isPressed);
  // Serial.printf("millis():%d,resettime():%d\n", millis(), resetTime);

  if(upButton.isPressed && downButton.isPressed)
  {
    if(millis() - resetTime > 2000)
    {
      if(secondLoop)
      {
        counter = 0;
        tallyReset();
      }
      secondLoop = true;
      resetTime = millis();
    }
  }
  else
  {
    secondLoop = false;
  }
  
}
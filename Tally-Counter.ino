/**************************************************************
 Code written by gigahidjrikaaa for course project.
 Based on the examples written by Limor Fried/Ladyada.

 This code is used for showing a tally counter (counter in general)
 using two push buttons with an SSD1306 128x64 OLED.

 Features:
 - 2 buttons which increment/decrement the counter when pressed
   individually, reset the counter when pressed together.
 - OLED displays the counter.
***************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // Obtained from I2C Scanner. Look for the program in Github.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button Class to manage all things related to buttons.
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
      buttonPin = pin; // assigns the button pin
      modifier = number; // what value is added to the counter
      counter = count; // counter variable address so this class can change the counter variable.
    }

    // Member function to add a value to the counter.
    void addCounter()
    {
      *counter += modifier;
      if(*counter < 0) // limits the counter so it cannot be a negative number.
        *counter = 0;
    }

    // Member function to always check the button.
    // Must be called at least once in the loop() function.
    void buttonCheck()
    {
      // read the current state of the button
      currentState = digitalRead(buttonPin);

      // Button debouncing
      // For more information on debouncing, check ESP32.io
      if(millis() - lastDebounceTime > debounceThres)
      {
        if(lastSteadyState == HIGH && currentState == LOW)
        {
          isPressed = true;
          addCounter(); // Only adds the counter when button is pressed (after the debouncing)
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

// Function to make the OLED displays content centered with x and y offset
void displayCenter(String text, int X, int Y) {
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  display.setCursor((SCREEN_WIDTH - width) / 2 + X, (SCREEN_HEIGHT - height) / 2 + Y);
  display.println(text);
}

// Function to show the counter to the OLED
void tallyCounter(int counter)
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE); // Set the Color of text
  display.setTextSize(2); // Set the text size
  displayCenter("Counter", 0, -24); // Display the text "Counter"
  display.setTextSize(4); 
  displayCenter(String(counter), 0, 8); // Display the counter value

  display.display();
}

// Function to show a RESET display to the OLED
void tallyReset()
{
  display.clearDisplay();
  display.setTextSize(4);
  displayCenter("RESET", 0, 0);
  display.display();
  delay(2000); // delay to not receive any input from buttons
}

const int pinUpBtn = 14;
const int pinDownBtn = 12;
int counter = 0;

unsigned long int resetTime = 0;
bool secondLoop = 0;

BtnClass upButton(pinUpBtn, 1, &counter);
BtnClass downButton(pinDownBtn, -1, &counter);

void setup() {
  Serial.begin(115200);

  // Checks if the OLED is connected or not.
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Shows the Adafruit splash screen
  display.display();
  delay(1000);
  display.clearDisplay();

  // Use internal pullups for buttons
  pinMode(pinUpBtn, INPUT_PULLUP);
  pinMode(pinDownBtn, INPUT_PULLUP);
}

void loop() {
  tallyCounter(counter);
  upButton.buttonCheck();
  downButton.buttonCheck();

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

  // For debugging the buttons only
  // Serial.printf("UpButtonState:%d,DownButtonState:%d,UpIsPressed:%d,DownIsPressed:%d\n", upButton.currentState, downButton.currentState, upButton.isPressed, downButton.isPressed);
  // Serial.printf("millis():%d,resettime():%d\n", millis(), resetTime);
}
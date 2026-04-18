#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>

byte maxScreenPointer = 3;
byte screenPointer = 0;
LiquidCrystal_I2C lcd(0x27,16,2);

//WiFi Setup
//=======================================================================
const char* ssid       = "WiFi_Name";
const char* password   = "WiFi_Password";

//PIN Setup
//=======================================================================
const byte buzzerPin = 14;
const byte buttonCount = 4;
const byte buttonPins[] = {4, 5, 13, 18}; //{left=4, right=5, select=13, cycle=18}



//Functions
//=======================================================================

/**
  Moves the screenPointer left if False or right if True.
*/
void moveScreen(bool direction){
  if(direction){
    screenPointer++;
    if(screenPointer > maxScreenPointer){ screenPointer = 0; }
  }
  else{
    if(screenPointer == 0){ screenPointer = maxScreenPointer; }
    else{ screenPointer--; }
  }
}

/**
  Button Listener:
  Performs Actions if buttons are pressed.
  NOTE: physical buttons are colored, RED, YELLOW, GREEN, & BLUE
*/
void readButtons() {
    for (byte i = 0; i < buttonCount; i++) {
      byte buttonPin = buttonPins[i];
      if (digitalRead(buttonPin) == LOW) {
        switch (buttonPin) {
          case 4:
            Serial.print("Red");
            break;

          case 5:
            Serial.print("Yellow");
            break;

          case 13:
            Serial.print("Green");
            break;

          case 18:
            Serial.print("Blue");
            break;

          default:
            Serial.print("Error");
            break;
        }
        Serial.println(" = LOW");
      }
    }
}


//Setup
//=======================================================================
void setup() {
  // put your setup code here, to run once:

}

//MAIN - Loop
//=======================================================================
void loop() {
  // put your main code here, to run repeatedly:

}
